
float maxPressureLimits[5] = {0,0,0,0,0};
float minPressureLimits[5] = {0,0,0,0,0};

float pressureOld = flowio.getPressure(PSI);

void createPressureService(void) {
  uint8_t pressureServiceUUID[16]     = {0x05,0xaa,0x00,0x00,0x00,0x00,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-00000000aa05"
  uint16_t chrPressureValueUUID = 0X2A6D; //"0X2A6D"
  uint8_t chrPressureRequestUUID[16]  = {0x05,0xaa,0x00,0x00,0x00,0xc2,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa05"
  uint8_t chrMaxPressureLimitsUUID[16]= {0x05,0xaa,0x00,0x00,0x00,0xc3,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c3000000aa05"
  uint8_t chrMinPressureLimitsUUID[16]= {0x05,0xaa,0x00,0x00,0x00,0xc4,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c4000000aa05"
  
  pressureService = BLEService(pressureServiceUUID);
  pressureService.begin();

  chrPressureValue = BLECharacteristic(chrPressureValueUUID);
  chrPressureValue.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrPressureValue.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPressureValue.setFixedLen(4);
  chrPressureValue.begin();

  chrPressureRequest = BLECharacteristic(chrPressureRequestUUID);
  chrPressureRequest.setProperties(CHR_PROPS_WRITE);
  chrPressureRequest.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPressureRequest.setFixedLen(1);
  chrPressureRequest.setWriteCallback(onWrite_chrPressureRequest);
  chrPressureRequest.begin();

  chrMaxPressureLimits = BLECharacteristic(chrMaxPressureLimitsUUID);
  chrMaxPressureLimits.setProperties(CHR_PROPS_WRITE | CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrMaxPressureLimits.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrMaxPressureLimits.setFixedLen(20); //20/5 = 4 bytes per port because a float is 4-byte.
  chrMaxPressureLimits.setWriteCallback(onWrite_chrMaxPressureLimits);
  chrMaxPressureLimits.begin();

  chrMinPressureLimits = BLECharacteristic(chrMinPressureLimitsUUID);
  chrMinPressureLimits.setProperties(CHR_PROPS_WRITE | CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrMinPressureLimits.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrMinPressureLimits.setFixedLen(20);
  chrMinPressureLimits.setWriteCallback(onWrite_chrMinPressureLimits);
  chrMinPressureLimits.begin();
}

//This is executed when a central device writes to the characteristic.
void onWrite_chrPressureRequest(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  Serial.println("Value Written");  
  if(len==1){
    float num = flowio.getPressure(PSI);
    chrPressureValue.notify(&num,sizeof(num));
  }
}
//The data array is 8-bit so I need to read 4 bytes and do some bit shifting. 

//The only utility of the onWrite functions is for me to populate my minLimits and maxLimits arrays.

void onWrite_chrMinPressureLimits(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  if(len==20){
    for(int i=0; i<5; i++){
      uint8_t bytes[4] = {data[0+i*4],data[1+i*4],data[2+i*4],data[3+i*4]};
      float x = *(float *)&bytes; //not clear how this works, but it does.
      minLimits[i]=x;
      Serial.println( minLimits[i]);
    }
  }
}
void onWrite_chrMaxPressureLimits(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  Serial.println("Max Pressure: ");
  if(len==20){
    for(int i=0; i<5; i++){
      uint8_t bytes[4] = {data[0+i*4],data[1+i*4],data[2+i*4],data[3+i*4]};
      float x = *(float *)&bytes;
      maxLimits[i]=x;
      Serial.println( maxLimits[i]);
    }
  }
}

//This function sends a pressure value notification if the device is in an active state, and if the pressure is different from last time.
void notifyPressure_IfActive_IfDeltaP(float dP){
  if(Bluefruit.connected()){ //This is mandatory here becasue we cannot execute .notify8() unless connected.
    uint16_t hardwareState = flowio.getHardwareState();
    uint8_t hardwareStateByte0 = (uint8_t) hardwareState;
    if(hardwareStateByte0>0){
      float pressure = flowio.getPressure(PSI);
      if(pressure < pressureOld-dP || pressure > pressureOld+dP){
        chrPressureValue.notify(&pressure,sizeof(pressure));
        pressureOld = pressure;
      }
    }      
  }
}
