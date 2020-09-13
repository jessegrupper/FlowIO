/*I can send the voltages as either a single 32bit value or an array of 4 bytes.
 * The latter approach makes the comparison between the old and new value more difficult,
 * because I have to compare each element in the array individually. 
 */
#define avg 30
#define sampleDelay 10

uint16_t initVoltagesAvg[4] = {0}; //each item holds the initial voltage, averaged over 30 samples.
uint16_t currentVoltagesAvg[4] = {0}; //each item holds the 12-bit voltage value. 
uint8_t normalizedVoltagesAvg8bit[4] = {0};
uint32_t voltagesUint32;
uint32_t strainVals32bit = 0; 
uint32_t strainVals32bitOld = 0; 
int lastTime = millis();


void createStrainService(){
  const uint8_t strainServiceUUID[16]     = {0x10,0xaa,0x00,0x00,0x00,0x00,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-00000000aa01"
  const uint8_t chrStrainUUID[16] = {0x10,0xaa,0x00,0x00,0x00,0xc1,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa01"

  strainService = BLEService(strainServiceUUID);
  strainService.begin();

  chrStrain = BLECharacteristic(chrStrainUUID);
  chrStrain.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY); 
  chrStrain.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    //For the permission, the first parameter is the READ, second the WRITE permission
  chrStrain.setFixedLen(4);
  chrStrain.begin();
}

void getVoltagesAveraged(uint16_t voltAvg[], uint8_t avgNum, uint8_t delayTime){
  //This function directly modifies the array passed to its argument. Arrays are always passed by reference in C.
  for(int i=0; i<avgNum; i++){
    voltAvg[0]+=analogRead(29);
    voltAvg[1]+=analogRead(5);
    voltAvg[2]+=analogRead(4);
    voltAvg[3]+=analogRead(3);
    delay(delayTime);
  }
  for(int i=0; i<4; i++){
    voltAvg[i]/=avgNum;
  }
}
uint8_t scale12to8bit(uint16_t val0to4095){ //the input is assumed to be b/n 0 and 4095 (12-bit only)
  float val0to255float = val0to4095*255.0/4095.0;
  return (uint8_t) (val0to255float+0.5); //casting to 8bit with rounding.
}
uint32_t arrayToUint32(uint8_t array4element[]){ //takes an 8-bit array of 4 elements, and produces a uint32. 
  uint32_t var;
  var = array4element[3];   //byte 3
  var <<= 8;
  var |= array4element[2];  //byte 2
  var <<= 8;
  var |= array4element[1];  //byte 1
  var <<= 8;
  var |= array4element[0];  //byte 0
  return var;
}

void updateStrainEvery(int interval){
  if(Bluefruit.connected()){ //This is mandatory here becasue we cannot execute .notify8() unless connected.
    if(millis() - lastTime > interval){ //we wiil check the battery only once per 5 seconds. This reduces power consumption greatly.
      getVoltagesAveraged(currentVoltagesAvg,avg,sampleDelay);
      for(int i=0; i<4; i++){
        normalizedVoltagesAvg8bit[i] = scale12to8bit(currentVoltagesAvg[i]) - scale12to8bit(initVoltagesAvg[i]);   
      }
      strainVals32bit = arrayToUint32(normalizedVoltagesAvg8bit);
      lastTime = millis();
      if(strainVals32bit != strainVals32bitOld){
        chrStrain.notify32(strainVals32bit);        
        Serial.println(strainVals32bit);
        strainVals32bitOld = strainVals32bit;
      }
    }
  }
}
//The following function is added, so that we can declare the array in this file not the main
//file. We cannot call the interior function directly from the main file b/c it needs an argument
//that is not declared in the main file.
void setInitVoltagesAvg(){
  getVoltagesAveraged(initVoltagesAvg, avg, sampleDelay); //modifies the array passed in the argument.
}
