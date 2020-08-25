//NOTE: Remember that you MUST execute 'resetOffTimer()' inside each event loop that you consider as non-idle event.

uint8_t inactiveTimeLimit[] = {5}; //minutes of inactivity until power off;
int offTimerStart = millis();
bool remaining1minuteSent=false;
bool remaining2minuteSent=false;
bool remaining3minuteSent=false;
uint8_t remainingTimeUint;
float remainingTime;
int timerLastChecked = millis();
uint16_t hardwareStateOld = flowio.getHardwareState();

void createPowerOffService(void) {
  const uint8_t powerServiceUUID[16]              = {0x01,0xaa,0x00,0x00,0x00,0x00,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-00000000aa01"
  const uint8_t chrPowerOffNowUUID[16]            = {0x01,0xaa,0x00,0x00,0x00,0xc1,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa01"
  const uint8_t chrPowerOffWhenInactiveForUUID[16]= {0x01,0xaa,0x00,0x00,0x00,0xc2,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa01"
  const uint8_t chrPowerOffRemainingTimeUUID[16]  = {0x01,0xaa,0x00,0x00,0x00,0xc3,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c3000000aa01"

  powerOffService = BLEService(powerServiceUUID);
  powerOffService.begin();

  chrPowerOffNow = BLECharacteristic(chrPowerOffNowUUID);
  chrPowerOffNow.setProperties(CHR_PROPS_WRITE);
  chrPowerOffNow.setWriteCallback(onWrite_chrPowerOffNow);
  chrPowerOffNow.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPowerOffNow.setFixedLen(1);
  chrPowerOffNow.begin();

  chrPowerOffWhenInactiveFor = BLECharacteristic(chrPowerOffWhenInactiveForUUID);
  chrPowerOffWhenInactiveFor.setProperties(CHR_PROPS_WRITE | CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  chrPowerOffWhenInactiveFor.setWriteCallback(onWrite_chrPowerOffWhenInactiveFor);
  chrPowerOffWhenInactiveFor.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPowerOffWhenInactiveFor.setFixedLen(1);
  chrPowerOffWhenInactiveFor.begin();
  chrPowerOffWhenInactiveFor.write(inactiveTimeLimit,1); //this MIUST go after .begin().

  chrPowerOffRemainingTime = BLECharacteristic(chrPowerOffRemainingTimeUUID);
  chrPowerOffRemainingTime.setProperties(CHR_PROPS_NOTIFY);
  chrPowerOffRemainingTime.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPowerOffRemainingTime.setFixedLen(1);
  chrPowerOffRemainingTime.begin();
  chrPowerOffRemainingTime.write(inactiveTimeLimit,1); //this MIUST go after .begin().
}

//This is executed when a central device writes to chrPowerOffNow characteristic.
void onWrite_chrPowerOffNow(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  if(len==1){
    if(data[0]==0x01) flowio.powerOFF();
  }
}
//This is executed when a central device writes to the chrPowerOffWhenInactiveFor characteristic
void onWrite_chrPowerOffWhenInactiveFor(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  resetOffTimer(); //reset whenever something is written, even if invalid entry
  if(len==1){ //If the data is only byte, only then write to the characteristic. 
    inactiveTimeLimit[0] = data[0];
    chrPowerOffWhenInactiveFor.notify8(data[0]); //send a notification with the data written.
  }
}

void resetOffTimer(){   //You must run this function in each event that you consider "active" event 
  offTimerStart = millis();
  Serial.println("RESET TIMER");
  remaining1minuteSent=false;
  remaining2minuteSent=false;
  remaining3minuteSent=false;
}
void checkIfTimeToPowerOffEvery(int interval){ //the argument is in milliseconds
  if(millis() - timerLastChecked > interval){ //this is to save resources. We want to chech only once every interval milliseconds.
    timerLastChecked = millis();
    if(inactiveTimeLimit[0]==0) return; //0 means that the feature is disabled.
    
    remainingTime = inactiveTimeLimit[0] - (millis() - offTimerStart)/60000.0f;
    
    if(remainingTime<=1 && remaining1minuteSent==false){
      Serial.println("1 minute till off");
      remaining1minuteSent=true; //this is to ensure that we execute this only once.
      if(Bluefruit.connected()) chrPowerOffRemainingTime.notify8(1);
    }
    else if(remainingTime>=1 && remainingTime<=2 && remaining2minuteSent==false){
      Serial.println("2 minutes till off");
      remaining2minuteSent=true;
      if(Bluefruit.connected()) chrPowerOffRemainingTime.notify8(2);
    }
    else if(remainingTime>=2 && remainingTime<=3 && remaining3minuteSent==false){
      Serial.println("3 minutes till off");
      remaining3minuteSent=true;
      if(Bluefruit.connected()) chrPowerOffRemainingTime.notify8(3);
    }
    else if(remainingTime <= 0){     //IF YOU GET THIS SIGN MIXED WITH >=, YOU WILL BRICK YOUR DEVICE!!!
      flowio.powerOFF();
      resetOffTimer(); //This is useful in case of a hardware failure and power is not cut off properly.
    }
    //Serial.println(remainingtime);

    //Check if there is a hardware change and reset the timer if yes.
    uint16_t hardwareState = flowio.getHardwareState();
    if(hardwareState != hardwareStateOld) resetOffTimer();
    hardwareStateOld = hardwareState;    
  }
}
