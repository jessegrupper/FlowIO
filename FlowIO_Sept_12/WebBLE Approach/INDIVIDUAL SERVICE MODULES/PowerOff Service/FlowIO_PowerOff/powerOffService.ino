//NOTE: Remember that you MUST execute 'restartOffTimer()' inside each event loop that you consider as non-idle event.

uint8_t inactiveTimeLimit[] = {5}; //minutes of inactivity until power off;
int offTimerStart = millis();
bool remaining1minuteSent=false;
bool remaining2minuteSent=false;
bool remaining3minuteSent=false;
uint8_t remainingTimeUint8;
float remainingTime;
int timerLastChecked = millis();
uint16_t hardwareStateOld = flowio.getHardwareState();

void createPowerOffService(void) {
  const uint8_t powerServiceUUID[16]     = {0x01,0xaa,0x00,0x00,0x00,0x00,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-00000000aa01"
  const uint8_t chrPowerOffTimerUUID[16] = {0x01,0xaa,0x00,0x00,0x00,0xc1,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa01"

  powerOffService = BLEService(powerServiceUUID);
  powerOffService.begin();

  chrPowerOffTimer = BLECharacteristic(chrPowerOffTimerUUID);
  chrPowerOffTimer.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE | CHR_PROPS_NOTIFY);
  chrPowerOffTimer.setWriteCallback(onWrite_chrPowerOffTimer);
  chrPowerOffTimer.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPowerOffTimer.setFixedLen(1);
  chrPowerOffTimer.begin();
  chrPowerOffTimer.write(inactiveTimeLimit, 1); //initializes the timer to 5 minutes. 
}

//This is executed when a central device writes to the characteristic:
void onWrite_chrPowerOffTimer(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len){
  restartOffTimer(); //reset whenever a value is written.
  if(len==1){
    if(data[0]==0x00) flowio.powerOFF();            //power off now.
    else if(data[0]==0xFF) inactiveTimeLimit[0]=0;  //Set the time limit to 0 to disable auto-off.
    else inactiveTimeLimit[0] = data[0];            //Set the time limit to the value written.
    chrPowerOffTimer.notify8(data[0]);      //Send a notification with the value written:
  }
}

void resetOffTimer(){   //Resets the timer to its initial value of 5 minutes. This is only used in the connect callback.
  restartOffTimer();
  inactiveTimeLimit[0] = 5;
  chrPowerOffTimer.write8(inactiveTimeLimit[0]);
}
void restartOffTimer(){   //Restarts the timer to whatever value was selected in the GUI 
  offTimerStart = millis();
  Serial.println("RESTART TIMER");
  remaining1minuteSent=false;
  remaining2minuteSent=false;
  remaining3minuteSent=false;
}
void checkIfTimeToPowerOffEvery(int interval){ //the argument is in milliseconds
  if(millis() - timerLastChecked > interval){ //this is to save resources. We want to chech only once every interval milliseconds.
    timerLastChecked = millis(); //this must be the first line to ensure that we get the time even if we exis out prematurely.
    
    //If the timeLimit is set to 0, feature is disabled, thus exit the function
    if(inactiveTimeLimit[0]==0) return; //0 means that the feature is disabled.
    
    //If there is a hardware change, restart the timer and exit the function.
    uint16_t hardwareState = flowio.getHardwareState();
    if(hardwareState != hardwareStateOld){
      restartOffTimer();
      hardwareStateOld = hardwareState;
      return;
    }
      
    //Comput the remaining time in minutes and write it to the characteristic. 
    remainingTime = inactiveTimeLimit[0] - (millis() - offTimerStart)/60000.0f;
    remainingTimeUint8 = (uint8_t) remainingTime; //TODO: would this conversation work as expected? Check what are the bits inside the uint variable.
    chrPowerOffTimer.write8(remainingTimeUint8);
    Serial.println(remainingTime);
    Serial.println(remainingTimeUint8);

    //Check if the remaining time is below our thresholds and do as needed.
    if(remainingTime<=1 && remaining1minuteSent==false){
      Serial.println("1 minute till off");
      remaining1minuteSent=true; //this is to ensure that we execute this only once.
      if(Bluefruit.connected()) chrPowerOffTimer.notify8(1);
    }
    else if(remainingTime>=1 && remainingTime<=2 && remaining2minuteSent==false){
      Serial.println("2 minutes till off");
      remaining2minuteSent=true;
      if(Bluefruit.connected()) chrPowerOffTimer.notify8(2);
    }
    else if(remainingTime>=2 && remainingTime<=3 && remaining3minuteSent==false){
      Serial.println("3 minutes till off");
      remaining3minuteSent=true;
      if(Bluefruit.connected()) chrPowerOffTimer.notify8(3);
    }
    else if(remainingTime <= 0){     //IF YOU GET THIS SIGN MIXED WITH >=, YOU WILL BRICK YOUR DEVICE!!!
      flowio.powerOFF();
      restartOffTimer(); //This is useful in case of a hardware failure and power is not cut off properly.
    }
    //Serial.println(remainingtime);

  
  }
}
