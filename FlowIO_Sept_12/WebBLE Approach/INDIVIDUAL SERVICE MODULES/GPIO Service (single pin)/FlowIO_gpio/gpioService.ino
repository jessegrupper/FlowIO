//TODO: Implement a GPIOstatus, similar to the hardware status byte, to show the current pin states. 
//WONTDO: If I am in PWM mode, it is actually impossible to go to any other mode. Maybe I should disable that switch in code as well. Or what I could
//do is if I am in PWM mode, and someone asks to do writeDigital(0) I should just modify that to writeAnalog(0) behind the scenes. But that will create 
//a hidden behavior, which is considered a bad practice, so I won't do it. 

uint8_t state02 = 0xFF; //this is the hex value of the state.
uint8_t mode02flag; //INPUT, OUTPUT, INPUT_PULLUP, PWM.
bool notify02flag = false; //If True, notifications are sent continuously when dV>5/1024.
uint16_t val02 = 0; //this is the pin value.
uint16_t val02old = 0; //this is the previous pin value.

void createGpioService(void) {
  uint8_t gpioServiceUUID[16]     = {0x06,0xaa,0x00,0x00,0x00,0x00,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-00000000aa06"
  uint8_t chrPin02UUID[16]= {0x06,0xaa,0x00,0x00,0x00,0xc1,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa06"
  
  gpioService = BLEService(gpioServiceUUID);
  gpioService.begin();

  chrPin02 = BLECharacteristic(chrPin02UUID);
  chrPin02.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY | CHR_PROPS_WRITE);
  chrPin02.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPin02.setFixedLen(2);
  chrPin02.setWriteCallback(onWrite_chrPin02);  
  chrPin02.begin();
}

//This is executed when a central device writes to the characteristic.
void onWrite_chrPin02(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  Serial.println("Value Written on chrPin02");  
  state02 = data[0];
  //The only classification that makes sense is digital, analog, pwm - because those are hardware constraints. 
  if(len==1 && state02<=0x05)                       setDigitalState(2, state02, chrPin02, mode02flag, notify02flag, val02);
  else if(len==1 && state02>0x05 && state02<= 0x09) setAnalogState(2, state02, chrPin02, mode02flag, notify02flag, val02);
  else if(len==2 && state02==0x0A)                  setPwmState(2, data[1], chrPin02, mode02flag, notify02flag, val02);
  else flowio.raiseError(2); //Invalid value written to chrPin02
}

//This function sends notification if the pin mode is in notify state, and if value is different from last time by dV.
void notifyInputValueChange_Pin02_ifdV(uint8_t dV){
  if(Bluefruit.connected() && notify02flag==true){ //This is mandatory here becasue we cannot execute .notify16() unless connected.
      if(state02==0x04 || state02==0x05)        val02 = digitalRead(2);      
      else if(state02==0x06 || state02==0x07)   val02 = analogRead(2);
      else flowio.raiseError(102);      
      if(val02 < val02old-dV || val02 > val02old+dV){
        chrPin02.notify(&val02,sizeof(val02)); //we can use notify16(val02) as well.
        val02old = val02;
      }
  }
}
void setDigitalState(uint8_t pin, uint8_t& state, BLECharacteristic& chrPin, uint8_t& modeFlag, bool& notifyFlag, uint16_t& val){    
    //--------- set the pin mode and flag ---------------
    if(state==0x00 || state==0x01){ //m stands for mode
      if(modeFlag!=OUTPUT){
        modeFlag=OUTPUT;
        pinMode(pin,OUTPUT);
      }
    }
    else if(state==0x02 || state==0x04){
      if(modeFlag!=INPUT){
        modeFlag=INPUT;
        pinMode(pin,INPUT);
      }
    }
    else if(state==0x03 || state==0x05){
      if(modeFlag!=INPUT_PULLUP){
        modeFlag=INPUT_PULLUP;
        pinMode(pin,INPUT_PULLUP);
      }
    }
    //--------- set the notify flag -------------
    if(state==0x04 || state==0x05) notifyFlag=true;
    else notifyFlag=false;
    //--------- set the pin state and notify value ---------------
    //NOTE: Because val is a reference to val##, we are actually modifying the original variable val## here.
    if(state==0x00){
      digitalWrite(pin,LOW);    
      val = 0;
    }
    else if(state==0x01){
      digitalWrite(pin,HIGH);
      val = 1;
    }
    else if(state>=0x02 && state<=0x05){
      val = digitalRead(pin);
    }
    chrPin.notify16(val);
}
void setAnalogState(uint8_t pin, uint8_t& state, BLECharacteristic& chrPin, uint8_t& modeFlag, bool& notifyFlag, uint16_t& val){    
    //--------- set the pin mode and flag ---------------
    if(state==0x06 || state==0x08){
      if(modeFlag!=INPUT){
        modeFlag=INPUT;
        pinMode(pin,INPUT);
      }
    }
    else if(state==0x07 || state==0x09){
      if(modeFlag!=INPUT_PULLUP){
        modeFlag=INPUT_PULLUP;
        pinMode(pin,INPUT_PULLUP);
      }
    }
    //--------- set the notify flag -------------
    if(state==0x06 || state==0x07) notifyFlag=true;
    else notifyFlag=false;
    //--------- set the pin state and notify value ---------------
    //NOTE: Because val is a reference to val##, we are actually modifying the original variable val## here.
    val = analogRead(pin);
    chrPin.notify16(val);
}
void setPwmState(uint8_t pin, uint8_t& pwmVal, BLECharacteristic& chrPin, uint8_t& modeFlag, bool& notifyFlag, uint16_t& val){    
    //--------- set the pin mode and flags ---------------
    if(modeFlag!=222){ //I will use 222 for PWM, b/c PWM is not a defined pin state.
      modeFlag=222;
      pinMode(pin,OUTPUT);
      notifyFlag=false;
    }
    //--------- set the pin state and notify value ---------------
    //NOTE: Because val is a reference to val##, we are actually modifying the original variable val## here.
    analogWrite(pin,pwmVal);
    val = (uint16_t) pwmVal;
    chrPin.notify16(val);
}
