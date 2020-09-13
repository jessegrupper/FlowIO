/*
 * Read the voltages on the 4 analog pins. Then map those voltages into just 1-byte.
 * Then do byte-shifting operation so that you write the characteristic a 32-bit value.
 * This would allow you to then use the notify32(uint32_t num) to send the data.
 */
#define VBAT_PIN          A7            //A7 for nrf52832, A6 for nrf52840
#define VBAT_RESOLUTION   0.73242188F   //For 12bit @ 3V reference, resolution is 3000mV/4096steps = 0.732421875mV/step.
#define VBAT_DIVIDER_COMP 1.403F        // this is 4.2/3
#define REAL_VBAT_RESOLUTION (VBAT_RESOLUTION * VBAT_DIVIDER_COMP) //this is in units of millivolts/step.


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

uint8_t getVoltage8bit(uint8_t pin) {
  int voltage0to2095 = analogRead(pin);   // Get the raw 12-bit, 0..3000mV ADC value  
  float voltage0to255float = voltage0to2095*255.0/4095.0;
  //A conversion from float to int trunkates the result. 
  //If we add 0.5 to the float value before the trunkating conversion, then we actually
  //achieve rouding. For example, say val=3.6. If we convert directly, it would become 3.
  //But if we add 0.5 first and then convert it becomes 4.1 and convert to 4.
  uint8_t voltage8bit = (uint8_t) (voltage0to255float+0.5);
  
//  Serial.print(voltage0to2095);
//  Serial.print("\t");
//  Serial.print(voltage0to255float);
//  Serial.print("\t");
//  Serial.println(voltage8bit);
  
  return voltage8bit; 
}

uint32_t getStrain32bit(){
  uint32_t strain32bit;
  strain32bit = getVoltage8bit(3);   //byte 3
  strain32bit <<= 8;
  strain32bit |= getVoltage8bit(4);  //byte 2
  strain32bit <<= 8;
  strain32bit |= getVoltage8bit(5);  //byte 1
  strain32bit <<= 8;
  strain32bit |= getVoltage8bit(28);  //byte 0
  return strain32bit;
}

void updateStrainEvery(int interval){
  if(Bluefruit.connected()){ //This is mandatory here becasue we cannot execute .notify8() unless connected.
    if(millis() - lastTime > interval){ //we wiil check the battery only once per 5 seconds. This reduces power consumption greatly.
      //Serial.println("Read Strain Value");
      strainVals32bit = getStrain32bit();
      //Serial.println(strainVals32bit);
      lastTime = millis();
      if(strainVals32bit < strainVals32bitOld-1 || strainVals32bit > strainVals32bitOld+1){
        chrStrain.notify32(strainVals32bit);
        Serial.println(strainVals32bit);
        strainVals32bitOld = strainVals32bit;
      }
    }
  }
}
