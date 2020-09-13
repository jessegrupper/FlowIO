//TODO: Send a notification with the pin value for the input modes.
//TODO: Change the notify function such that it accepts the pin number as an argument. You should then also change the whose service in similar ways.
//You can use an array for the pin numbers and for all the variables.
//TODO: Look at the pressure service and implement the continuous notifications on change in the same way as it is there.
//TODO: Assign a bit from the hardware status byte to analog pins. If a pin is set to output HIGH, the status bit shoult be 1.
//TODO: Does the byte length of a characteristic mean that the characteristic can send data of only that length? Probably no. This is only 
  //the the write length of the characteristic, and how much data you can write to it.

#define pin30 30

bool notifyOnChange = false; //in ms
uint8_t pin30Mode = 0xFF;
uint16_t pin30Val = 0;
uint16_t pin30ValOld = 0;
uint8_t pinModeFlag;

void createGpioService(void) {
  uint8_t gpioServiceUUID[16]     = {0x06,0xaa,0x00,0x00,0x00,0x00,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-00000000aa06"
  uint8_t chrPin30UUID[16]= {0x06,0xaa,0x00,0x00,0x00,0xc1,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b}; //"0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa06"
  
  gpioService = BLEService(gpioServiceUUID);
  gpioService.begin();

  chrPin30 = BLECharacteristic(chrPin30UUID);
  chrPin30.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY | CHR_PROPS_WRITE);
  chrPin30.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  chrPin30.setFixedLen(2);
  chrPin30.setWriteCallback(onWrite_chrPin30);  
  chrPin30.begin();
}

//This is executed when a central device writes to the characteristic.
void onWrite_chrPin30(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  Serial.println("Value Written");  
  if(len==1){
    switch(pin30Mode=data[0]){
      case 0x00: //writeDigital(30,0);
        if(pinModeFlag!=OUTPUT){
          pinMode(pin30,OUTPUT);
          pinModeFlag=OUTPUT;
        }
        digitalWrite(pin30,LOW);
        notifyOnChange = false;
        Serial.println("writeDigital(30,0);");
        break;
      case 0x01: //writeDigital(30,1);
        if(pinModeFlag!=OUTPUT){
          pinMode(pin30,OUTPUT);
          pinModeFlag=OUTPUT;
        }
        digitalWrite(pin30,HIGH);
        notifyOnChange = false;
        Serial.println("writeDigital(30,1);");
        break;
      case 0x02: //readDigital(30,'INPUT');
        if(pinModeFlag!=INPUT){
          pinMode(pin30,INPUT);
          pinModeFlag=INPUT;
        }
        pin30Val = digitalRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = false;
        Serial.println("readDigital(30,'INPUT');");
        Serial.println(digitalRead(30));
        break;
      case 0x03: //readDigital(30,'INPUT_PULLUP');
        if(pinModeFlag!=INPUT_PULLUP){
          pinMode(pin30,INPUT_PULLUP);
          pinModeFlag=INPUT_PULLUP;
        }
        pin30Val = digitalRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = false;
        Serial.println("readDigital(30,'INPUT_PULLUP');");
        Serial.println(digitalRead(30));
        break;
      case 0x04: //readAnalog(30,'INPUT');
        if(pinModeFlag!=INPUT){
          pinMode(pin30,INPUT);
          pinModeFlag=INPUT;
        }
        pin30Val = analogRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = false;
        Serial.println("readAnalog(30,'INPUT');");
        Serial.println(pin30Val);
        break;
      case 0x05: //readAnalog(30,'INPUT_PULLUP');
        if(pinModeFlag!=INPUT_PULLUP){
          pinMode(pin30,INPUT_PULLUP);
          pinModeFlag=INPUT_PULLUP;
        }
        pin30Val = analogRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = false;
        Serial.println("readAnalog(30,'INPUT_PULLUP');");
        Serial.println(pin30Val);
        break;
       case 0x06: //readDigitalNotify(30,'INPUT');
        if(pinModeFlag!=INPUT){
          pinMode(pin30,INPUT);
          pinModeFlag=INPUT;
        }
        pin30Val = digitalRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = true;
        Serial.println("readDigitalNotify(30,'INPUT');");
        break;
      case 0x07: //readDigitalNotify(30,'INPUT_PULLUP');
        if(pinModeFlag!=INPUT_PULLUP){
          pinMode(pin30,INPUT_PULLUP);
          pinModeFlag=INPUT_PULLUP;
        }
        pin30Val = digitalRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = true;
        Serial.println("readDigitalNotify(30,'INPUT_PULLUP');");
        break;
      case 0x08: //readAnalogNotify(30,'INPUT');
        if(pinModeFlag!=INPUT){
          pinMode(pin30,INPUT);
          pinModeFlag=INPUT;
        }
        pin30Val = analogRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = true;
        Serial.println("readAnalogNotify(30,'INPUT');");
        break;
      case 0x09: //readAnalogNotify(30,'INPUT_PULLUP');
        if(pinModeFlag!=INPUT_PULLUP){
          pinMode(pin30,INPUT_PULLUP);
          pinModeFlag=INPUT_PULLUP;
        }
        pin30Val = analogRead(30);
        chrPin30.notify16(pin30Val);
        notifyOnChange = true;
        Serial.println("readAnalogNotify(30,'INPUT_PULLUP');");
        break;
    }
  }
  if(len==2){ //this should only be true in the PWM mode.
    if(data[0]=0x0A){ //writePWM(30,data[1]);
      pinMode(pin30,OUTPUT);
      analogWrite(pin30,data[1]);
      chrPin30.notify16(data[1]);
      Serial.println("Pin30: mode PWM");
    }
    else{
      Serial.println("Invalid value written");
    }
  }
}

//This function sends notification if the pin mode is in notify state, and if value is different from last time by dV.
void notifyChrPin30_ifActive_ifDelta(uint8_t dV){
  if(Bluefruit.connected() && notifyOnChange==true){ //This is mandatory here becasue we cannot execute .notify16() unless connected.
      if(pin30Mode==0x02 || pin30Mode==0x03 || pin30Mode==0x06 || pin30Mode==0x07){
        pin30Val = digitalRead(30);
      }
      else if(pin30Mode==0x04 || pin30Mode==0x05 || pin30Mode==0x08 || pin30Mode==0x09){
        pin30Val = analogRead(30);
      }      
      if(pin30Val < pin30ValOld-dV || pin30Val > pin30ValOld+dV){
        chrPin30.notify(&pin30Val,sizeof(pin30Val));
        Serial.println("Change notification sent");
        pin30ValOld = pin30Val;
      }
  }
}
