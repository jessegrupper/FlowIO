#include "Arduino.h"
#include "FlowIO.h"
/*
In the GENERAL configuration the release valve is outlet valve. 
In the INFLATION_SERIES || INFLATION_PARALLEL configuration the release valve is the outlet valve
In the VACUUM_SERIES || VACUUM_PARALLEL configuration the release valve is the inlet valve.
In the API functions, we can only use InletValve and OutletValve terms, only in action categorization we can use the term 'release'.
*/

//CONSTRUCTOR: Assigns the pins as outputs. 
	//NOTE(1): The parameter variables have a scope limited to the consturctor. If we want to use some of those parameter
	//values that are passed by the user in a function other than the constructor, then we must save those values into
	//global variables (which we define with _underscore by convention) which have a scope not limited to the consturctor.
	//NOTE(2): There is a much more fundamental reason we need the these global member variables, and that has to do with
	//how they are stored in memory. When we create a member variable, a pointer is created that points to that variable. 
	FlowIO::FlowIO(){
		_setConfig(GENERAL);
	}
	FlowIO::FlowIO(Configuration mode){
		_setConfig(mode);	
	}

//Hardware Configuration Mode
	//pin5=leftmost valve, pin30=rightmost valve
void FlowIO::_setConfig(Configuration config){
	_config = config;
	pinMode(_inletValvePin, OUTPUT);
	digitalWrite(_inletValvePin,LOW);
	pinMode(_outletValvePin, OUTPUT);
	digitalWrite(_outletValvePin,LOW);
	for(int i=0; i < 2; i++){
		pinMode(_pumpPins[i], OUTPUT);
		digitalWrite(_pumpPins[i],LOW);
	}		
	for(int i=0; i < 5; i++){
		pinMode(_portValvePins[i], OUTPUT);
		digitalWrite(_portValvePins[i],LOW);
	}
	pinMode(blueLEDpin,OUTPUT);
	pinMode(redLEDpin,OUTPUT);
}
void FlowIO::setConfig(Configuration config){
	_config = config;
}
Configuration FlowIO::getConfig(){
	return _config;
}

//Sensor:
void FlowIO::setPressureUnit(Unit pUnit){ //unit is an enum defined in header file.
	_pressureUnit=pUnit;
}
bool FlowIO::activateSensor(){
  	_i2c = &Wire; //_i2c is a pointer that points to &Wire.
  	_i2c->begin();
  	_i2cInitialized=true; //Once this is set to true, there is nothing to set it back to false.
  		//this is the only problem with this, apparent only if someone calls the i2c .end() funtion
  		//and then tries to call the .getPressure() function without calling activateSensor() beforehand.
  		//In tht case, the system will hang, but a user would typically never need to ever call .end().
	delay(10);	
  	statusByte = _getStatusByte();
  	if(statusByte == 0b01000000) return true;
  	else return false;
}

uint8_t FlowIO::_getStatusByte(){
  _i2c->requestFrom(_addr, (uint8_t)1);
  return _i2c->read();
}

uint32_t FlowIO::_getRawPressure(){
  //Request a new pressure reading to be performed.
  _i2c->beginTransmission(_addr);
  _i2c->write(0xAA);   // command to read pressure
  _i2c->write((byte)0x0);
  _i2c->write((byte)0x0);
  _i2c->endTransmission();
    
  //At this point, the sensor has been asked to do a pressure reading. We can now send as many read requests as we
  //want and we will keep getting the same  pressure value. The sensor will keep spitting the same value until a 
  //a new pressure reading request is made. Now, we don't want to send a request to read all 4 bytes, because the 
  //sensor may still be buzy, so we want to just read the status byte first. And then after we have confirmed that 
  //it is not buzy, then we will send a request to read 4 bytes. Thus, the minimum number of bytes we will actually
  //read will be 5. This takes advantage of the fact that the sensor stops sending data when it sees the stop signal,
  //and then it starts sending data from byte 1 again, when a new read request is made. 
  //(An alternative approach would be to simply request 4 bytes from the start, and if the sensor is busy send another
  //request for 4 bytes. In the case when sensor is not buzy, this saves us from reading status byte twice, but in the
  //case when the sensor is buzy, we are reading the last 3 bytes when we know they are invalid. Thus this approach
  //would be more wasteful.)

  //Request to read the status byte only. Keep reading it until it is not busy.:
  while(statusByte=_getStatusByte() & MPRLS_BUSY){
    delay(2); //computation will will not take more than 5ms.
  }

  //Request to read all 4 bytes, including status byte of course.
  _i2c->requestFrom(_addr, (uint8_t)4);
  
  statusByte = _i2c->read();
  if (statusByte & MPRLS_SATURATION) return 0xFFFFFFFF;
  if (statusByte & MPRLS_INTEGRITY_FAIL)  return 0xFFFFFFFF;

  uint32_t rawData; //only the lower 24 bits will store our data. Upper 8 bits will be 0.
  rawData = _i2c->read();
  rawData <<= 8;
  rawData |= _i2c->read(); 
  rawData <<= 8;
  rawData |= _i2c->read();
  return rawData;
}

float FlowIO::getPressure(){
	if(_i2cInitialized==false) return 8888.0f;
	if(_pressureUnit==HPA) 		return _getPressureHPA();
	if (_pressureUnit==ATM) 	return _getPressureATM();
	if (_pressureUnit==PSI)	return _getPressurePSI();
}

float FlowIO::getPressure(Unit pUnit){
	if(_i2cInitialized==false) return 8888.0f;
  	if(pUnit==PSI) return _getPressurePSI();
  	if(pUnit==HPA) return _getPressureHPA();
  	if(pUnit==ATM) return _getPressureATM();
}

float FlowIO::_getPressureHPA(){
  return _getPressurePSI() * 68.947572932;
}

float FlowIO::_getPressurePSI(){
  uint32_t raw_psi = _getRawPressure();
  if (raw_psi == 0xFFFFFFFF) return NAN;

  //See Derivation in the google Docs file: FlowIO/PressureFlowIO + API
  float psi = raw_psi*(25/13421772.8)-3.125;
  return psi;
}

float FlowIO::_getPressureATM(){
  return _getPressurePSI() * 0.0680459639;
}

//DRIVER: functions to control individual elements
	//It is critical that ABSOLUTELY NEVER access a negative array element, because
	//we will get content in the previous memory location right before the array!
	//If pumpNumber=0 and we subtract 1, that would be bad, and could lead to physical damage because the 
	//if that memory happens to hold a valid IO pin number, that could be disastrous! 
	//This bug is very difficult to notice becase it may cause things to behave badly only sometimes.

	void FlowIO::startPump(uint8_t pumpNumber){			
	  if(pumpNumber==1 || pumpNumber==2)
		digitalWrite(_pumpPins[pumpNumber-1],HIGH); 	
	}												
	void FlowIO::stopPump(uint8_t pumpNumber){	
	  if(pumpNumber==1 || pumpNumber==2)
	  	digitalWrite(_pumpPins[pumpNumber-1],LOW);
	}
	void FlowIO::openInletValve(){
	  digitalWrite(_inletValvePin,HIGH);
	}
	void FlowIO::closeInletValve(){
	  digitalWrite(_inletValvePin,LOW);
	}	
	void FlowIO::openOutletValve(){
	  digitalWrite(_outletValvePin,HIGH);
	}
	void FlowIO::closeOutletValve(){
	  digitalWrite(_outletValvePin,LOW);
	}
	void FlowIO::setPorts(uint8_t ports){
      //we use the 5 least significant bits in the order from left to right.
      char portsArray[5] = {ports>>4 & 0x01, ports>>3 & 0x01, ports>>2 & 0x01, ports>>1 & 0x01, ports & 0x01};
      for(int i=0; i<5; i++){
        digitalWrite(_portValvePins[i],portsArray[i]); //element 0 of the pins array is port 1.
      }
  	}
  	void FlowIO::openPorts(uint8_t ports){ //opens only the ports having a 1 in their bits. Leaves the others unchanged.
  		char portsArray[5] = {ports>>4 & 0x01, ports>>3 & 0x01, ports>>2 & 0x01, ports>>1 & 0x01, ports & 0x01};
		for(int i=0; i<5; i++){
			if(portsArray[i]==0x01){ //only act on the ports that are 1. We want to set them to 1.
				digitalWrite(_portValvePins[i], 1); //element 0 of the pins array is port 1.
			}	
		}
  	}
  	void FlowIO::closePorts(uint8_t ports){ //closes only the ports with a 1 in the arugment byte. Leaves the others unchanged.
		char portsArray[5] = {ports>>4 & 0x01, ports>>3 & 0x01, ports>>2 & 0x01, ports>>1 & 0x01, ports & 0x01};
		for(int i=0; i<5; i++){
			if(portsArray[i]==0x01){ //only act on the ports that are 1. We want to set them to 0.
				digitalWrite(_portValvePins[i], 0); //element 0 of the pins array is port 1.
			}	
		}	
	}
  	void FlowIO::blueLED(bool power){
		digitalWrite(blueLEDpin,power);
	}
	void FlowIO::redLED(bool power){
		digitalWrite(redLEDpin,power);
	}
	void FlowIO::powerOFF(){
		pinMode(powerOFFpin,OUTPUT);
		digitalWrite(powerOFFpin,HIGH);
	}
  	

//COMMUNICATION API: Two character based control. This is an implementation of our communication protocol at the library level. By invoking the "command()" 
	//function you are able to do everything that API2 is able to do, and more, just based on what arguments you provide. The first argument 'action' is 
	//describing what to do (+,-,!,P,N,o). The second argument is describing on which ports that action is to be performed based on the position and 
	//value of the individual bits, namely the first 5 bits.
	//Maybe I can just put the state machine here from the Arduino code. For now I could do this in the unaltered way, using the existing protocol, and then later
	//I will just replace the protocol with the shortened protocol. 

	//Implementation of the control state machine (TUTORIAL ON STATE MACHINES: https://hackingmajenkoblog.wordpress.com/2016/02/01/the-finite-state-machine/)
	float FlowIO::command(uint8_t action, uint8_t ports){ //returns the internal pressure
		//Here I am going to extract information from the port parameter's bits.
		float pressure = 999.9f;
		switch(action){
			case '!':
				stopAction(ports);
				break;
			case '+':
				startInflation(ports); //if the config state is not right, this won't do anything, in which case we also don't want to read pressure.
				break;
			case '-':
				startVacuum(ports);
				break;
			case 'p': //inflate2x
				startInflationHalfCapacity(ports);
				break;
			case 'n': //vacuum2x
				startVacuumHalfCapacity(ports); 
				break;
			case '^':
				startRelease(ports);
				break;
			case 'o':
				openPorts(ports);
				break;
			case 'c':
				closePorts(ports);
				break;
			case 'f':
				if(ports=='f') powerOFF(); //power off when 'oo' is sent.
				break;
			case 'r':
				redLED(ports); //if port>0 led=on, if port=0 led=off.
				break;
			case 'b':
				blueLED(ports);
				break;
			case '?':
				if(ports=='?'){
					pressure = getPressure();
					break;
				}else{					
					closeInletValve();
					closeOutletValve();
					stopPump(1);
					stopPump(2);
					setPorts(ports);
					delay(10); //8ms is too little. 9ms is maybe Ok. 10ms is safe.
					pressure = getPressure();
					break;
				} 
				
		}
		//We want to get the pressure only in some states and NOT in others. And we want to return 0
		//for pressure if a command like 'r' is sent. We return a valid presure only for commands
		//that make sense to return a pressure reading.
		//There is still a bit of an issue because no matter in what hardware configuration I am in,
		//I will be getting the pressure value even if I call a function that does not correspond to 
		//that hardware configuration. So I could put those inside an if block just like the API2 functions
		//have if blocks that specify the actions for the different hardware configurations. 

		return pressure;
	}

//PROGRAMMING API: functions to perform actions involving more than one element.
	void FlowIO::startInflation(uint8_t ports){ 
		stopAction(ports);
	  	if(_config == GENERAL || _config == INFLATION_PARALLEL || _config == INFLATION_SERIES){
		 	if(ports<<3 != 0){ //if the ports are all 0, then don't do anything
		  		openInletValve(); 	//(If we don't have an inlet valve, it is ok b/c then this will act on pin "0")
		  		setPorts(ports);
		  		startPump(1);
		  		if(_config != GENERAL) startPump(2);
		  	}
		}
	  	
	}
	void FlowIO::startInflationHalfCapacity(uint8_t ports){ //Only applicable for a DualInflation system.
	  	stopAction(ports);
	  	if(_config == INFLATION_PARALLEL || _config == INFLATION_SERIES){
		  	if(ports<<3 != 0){
		  		openInletValve(); 
		  		setPorts(ports);
		  		startPump(1);
		  	}
	  	}
	}
	void FlowIO::startVacuum(uint8_t ports){ 
		stopAction(ports); 
		if(_config == GENERAL || _config == VACUUM_PARALLEL || _config == VACUUM_SERIES){
			if(ports<<3 != 0){
				openOutletValve();
				setPorts(ports);
				startPump(2);
				if(_config != GENERAL) startPump(1);
			}
		}
	}
	void FlowIO::startVacuumHalfCapacity(uint8_t ports){ //Only applicable for a DualVacuum system.
		stopAction(ports);
		if(_config == VACUUM_SERIES || _config == VACUUM_PARALLEL){
			if(ports<<3 != 0){	
				openOutletValve();
				setPorts(ports);  
				startPump(1);
			}
		}
	}
	void FlowIO::startRelease(uint8_t ports){
		stopAction(ports);		
		if(ports<<3 != 0){		
			setPorts(ports);
			if(_config == GENERAL || _config == INFLATION_PARALLEL || _config == INFLATION_SERIES)
				openOutletValve();  
			else{
				openInletValve();
			}
		}
	}
	void FlowIO::stopAction(uint8_t ports){
		closeInletValve();
		closeOutletValve();
		//If we are stopping the action on port 5, we naturally would type stopAction(0b00000001), which means that
		//we want to CLOSE port 5. BUT we don't want to change any of the other 4 ports. Thus we use closePorts rather than setPorts().
		closePorts(ports); //this only closes the ports indicated with a 1.
	  	stopPump(1);
		stopPump(2);
	}	

//API: functions to perform time-dependent actions (these are based on the above, and not on the driver directly) 
//------NOTE: THESE FUNCTIONS ARE BLOCKING EXECUTION (they include delay() elements.)
  	void FlowIO::inflateT(uint8_t ports, int millisec){ //this is a blocking function
		if(_config == GENERAL || _config == INFLATION_PARALLEL || _config == INFLATION_SERIES){
			if(millisec > 0 && ports<<3 != 0){
				startInflation(ports);
				delay(millisec);
				stopAction(ports);
			}
		}
	}
	void FlowIO::releaseT(uint8_t ports, int millisec){ //this is a blocking function
		if(millisec > 0 && ports<<3 != 0){
			startRelease(ports);
			delay(millisec);
			stopAction(ports);
		}
	}
	void FlowIO::vacuumT(uint8_t ports, int millisec){ //this is a blocking function
		if(_config == GENERAL || _config == VACUUM_PARALLEL || _config == VACUUM_SERIES){
			if(millisec > 0 && ports<<3 != 0){
				startVacuum(ports);
				delay(millisec);
				stopAction(ports);
			}
		}	
	}

	//API: Presure Dependent Methods (BLOCKING)
	
	//I will require pressure unit to be specified explicity, because otherwise user may forget what units
	//they had set and accidentally start working in different units, causing something to blow up. Moreover,
	//when units are specified explicity, this enables one to have different inflation operations happening in
	//different units, say if someone has inflatables that are rated at different pressure systems.
	void FlowIO::inflateP(uint8_t ports, float pMax, Unit pUnit){ //the value of pMax is of unit as specified in the units function.
		if(getPressure(pUnit)<pMax) startInflation(ports);
		while(getPressure(pUnit)<pMax){
			delay(1);
		}
		stopAction(ports);		
	}