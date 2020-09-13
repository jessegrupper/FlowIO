//TODO: Instead of a 32bit int value, can't I have an 8-bit int array?

/*We will do the averaging on the 12-bit voltage value but the normalizaton on the 8-bit value.
 */
#define avg 30
#define sampleDelay 10

uint16_t initVoltagesAvg[4] = {0}; //each item holds the initial voltage, averaged over 30 samples.
uint16_t currentVoltagesAvg[4] = {0}; //each item holds the 12-bit voltage value. 
uint8_t normalizedVoltagesAvg8bit[4] = {0};
uint32_t voltagesUint32;

void setup() {
  Serial.begin(115200);
  analogReference(AR_INTERNAL_3_0);   // Set the analog reference to 3.0V (default = 3.6V) b/c battery voltage may drop to <3.6V and then default will become inaccurate.
  analogReadResolution(12);   // Set the resolution to 12-bit (0..4095). Can be 8, 10, 12 or 14
  delay(10);
  getVoltagesAveraged(initVoltagesAvg, avg, sampleDelay); //modifies the array passed in the argument.
}

void loop() {
//  Serial.println(getVoltages32bit(),BIN);
  delay(1000);
  getVoltagesAveraged(currentVoltagesAvg,avg,sampleDelay);
  for(int i=0; i<4; i++){
    normalizedVoltagesAvg8bit[i] = scale12to8bit(currentVoltagesAvg[i]) - scale12to8bit(initVoltagesAvg[i]);   
  }
  Serial.println(arrayToUint32(normalizedVoltagesAvg8bit));
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

////I must do averaging of the 12-bit voltage values. 
//uint32_t getVoltages32bit(){
//  uint32_t voltages32bit = 0;
//  voltages32bit = scale12to8bit(analogRead(3)); //
//}
//
//uint8_t getVoltage8bit(uint8_t pin) {
//  int voltage0to2095 = analogRead(pin);   // Get the raw 12-bit, 0..3000mV ADC value  
//  float voltage0to255float = voltage0to2095*255.0/4095.0;
//  //A conversion from float to int trunkates the result. 
//  //If we add 0.5 to the float value before the trunkating conversion, then we actually
//  //achieve rouding. For example, say val=3.6. If we convert directly, it would become 3.
//  //But if we add 0.5 first and then convert it becomes 4.1 and convert to 4.
//  uint8_t voltage8bit = (uint8_t) (voltage0to255float+0.5);
//  
//  Serial.print(voltage0to2095);
//  Serial.print("\t <--->\t");
//  Serial.println(voltage8bit);
//  
//  return voltage8bit; 
//}
//
//uint32_t getVoltages32bit(){
//  uint32_t strain32bit;
//  strain32bit = getVoltage8bit(3);   //byte 3
//  strain32bit <<= 8;
//  strain32bit |= getVoltage8bit(4);  //byte 2
//  strain32bit <<= 8;
//  strain32bit |= getVoltage8bit(5);  //byte 1
//  strain32bit <<= 8;
//  strain32bit |= getVoltage8bit(29);  //byte 0
//  return strain32bit;
//}

//TODO: test whether the output value is actually the correct voltage.  I need 
//and extern la power source. If it is not, then I need to adjust hte analogReference.
