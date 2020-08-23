#include "Arduino.h"
#include "FlowIO.h"

//INDICATORS APIs
  	void FlowIO::blueLED(bool power){
		digitalWrite(blueLEDpin,power);
		(power) ? set_bit(10,_hardwareState) : clear_bit(10,_hardwareState);
	}
	void FlowIO::redLED(bool power){
		digitalWrite(redLEDpin,power);
		(power) ? set_bit(9,_hardwareState) : clear_bit(9,_hardwareState);		
	}
	void FlowIO::powerOFF(){
		pinMode(powerOFFpin,OUTPUT);
		digitalWrite(powerOFFpin,HIGH);
	}
  	uint16_t FlowIO::getHardwareState(){
  		return _hardwareState;
  	}
  	uint8_t FlowIO::readError(){
  		return _errorCode;
  	}
  	void FlowIO::raiseError(uint8_t errorCode){
  		_errorCode = errorCode;
  	}