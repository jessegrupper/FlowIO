#include "Arduino.h"
#include "FlowIO.h"

//CONSTRUCTOR: Assigns the pins as outputs. 
	//NOTE(1): The parameter variables have a scope limited to the consturctor. If we want to use some of those parameter
	//values that are passed by the user in a function other than the constructor, then we must save those values into
	//global variables (which we define with _underscore by convention) which have a scope not limited to the consturctor.
	//NOTE(2): There is a much more fundamental reason we need the these global member variables, and that has to do with
	//how they are stored in memory. When we create a member variable, a pointer is created that points to that variable. 
	FlowIO::FlowIO(){
		_portsInUse = 5;	
		_setConfigMode(ALL_PURPOSE);
	}
	FlowIO::FlowIO(uint8_t portsInUse){
		_portsInUse = portsInUse;	
		_setConfigMode(ALL_PURPOSE);	
	}
	FlowIO::FlowIO(uint8_t portsInUse, configuration mode){
		_portsInUse = portsInUse;	
		_setConfigMode(mode);	
	}

//Hardware Configuration Mode
	//pin5=leftmost valve, pin30=rightmost valve
void FlowIO::_setConfigMode(configuration mode){
	if(mode==ALL_PURPOSE){ //AllPurpose
		_inf=true;
		_vac=true;
		_inletValvePin=30;
		_releaseValvePin=5;
		_portValvePins[5]=0; //seting 6th element of the array to 0
	}
	else if(mode==INFLATIONPP){	//DualInflation (Direct to manifold connection)
		_inf=true;
		_vac=false;
		_inletValvePin=0; //no inlet valve
		_releaseValvePin=5; //leftmost valve
		_portValvePins[5]=30; //seting 6th element of the array to pin 30
	}
	pinMode(_inletValvePin, OUTPUT);
	digitalWrite(_inletValvePin,LOW);
	pinMode(_releaseValvePin, OUTPUT);
	digitalWrite(_releaseValvePin,LOW);
	for(int i=0; i < 2; i++){
		pinMode(_pumpPins[i], OUTPUT);
		digitalWrite(_pumpPins[i],LOW);
	}		
	for(int i=0; i < _portsInUse; i++){
		pinMode(_portValvePins[i], OUTPUT);
		digitalWrite(_portValvePins[i],LOW);
	}
	pinMode(blueLEDpin,OUTPUT);
	pinMode(redLEDpin,OUTPUT);
}


//Sensor:
void FlowIO::setPressureUnit(unit pUnit){ //unit is an enum defined in header file.
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
	if(_i2cInitialized==false) return 3.333f;
	if(_pressureUnit==HPA) 		return _getPressureHPA();
	if (_pressureUnit==ATM) 	return _getPressureATM();
	if (_pressureUnit==PSI)	return _getPressurePSI();
}

float FlowIO::getPressure(unit pUnit){
	if(_i2cInitialized==false) return 3.333f;
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
	void FlowIO::openReleaseValve(){
	  digitalWrite(_releaseValvePin,HIGH);
	}
	void FlowIO::closeReleaseValve(){
	  digitalWrite(_releaseValvePin,LOW);
	}
	//For the following 2 functions, it is critical that portNumber ABSOLUTELY NEVER be 0!
	//If it is 0, then we will be accessing the array _portValvePins[] at index -1, which 
	//is a memory location we should never be accessing. If that memory happens to hold a 
	//valid IO Pin, then we will be setting that pin to HIGH, which could be disastrouns!
	//In fact, it turns out that _portValvePins[-1]=_pumpPins[1]=3 which is the pin for the
	//second pump! 
	void FlowIO::openPort(uint8_t portNumber){ //port #1 corresponds to _portValvePins[0], thus.
	  if(portNumber>0 && portNumber<=MAXPORTS)	//making sure that port number is not 0.
	  	digitalWrite(_portValvePins[portNumber-1],HIGH); // we subtract 1 from portNumber.
	}
	void FlowIO::closePort(uint8_t portNumber){
	  if(portNumber>0 && portNumber<=MAXPORTS)	
	  	digitalWrite(_portValvePins[portNumber-1],LOW);
	}
	void FlowIO::openAllPorts(){
	  for(uint8_t i=0; i < _portsInUse; i++){
	    digitalWrite(_portValvePins[i],HIGH);
	  }
	}
	void FlowIO::closeAllPorts(){
	  for(uint8_t i=0; i < _portsInUse; i++){
	    digitalWrite(_portValvePins[i],LOW);
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

//NEW DRIVER
  void FlowIO::setPorts(uint8_t ports){
      //we use the 5 least significant bytes in the order from left to right.
      char portsArray[5] = {ports>>4 & 0x01, ports>>3 & 0x01, ports>>2 & 0x01, ports>>1 & 0x01, ports & 0x01};
      for(int i=0; i<5; i++){
        digitalWrite(_portValvePins[i],portsArray[i]); //element 0 of the pins array is port 1.
      }
  }

//API1: Two character based control. This is an implementation of our communication protocol at the library level. By invoking the "command()" 
	//function you are able to do everything that API2 is able to do, and more, just based on what arguments you provide. The first argument 'action' is 
	//describing what to do (+,-,!,P,N,o). The second argument is describing on which ports that action is to be performed based on the position and 
	//value of the individual bits, namely the first 5 bits.
	//Maybe I can just put the state machine here from the Arduino code. For now I could do this in the unaltered way, using the existing protocol, and then later
	//I will just replace the protocol with the shortened protocol. 

	//Implementation of the control state machine
	float FlowIO::sendCommand(uint8_t action, uint8_t port){ //returns the internal pressure
		//Here I am going to extract information from the port parameter's bits.
		float pressure = 0.0;
		switch(action){
			case '!':
				(port==0) ? stopActionAll() : stopAction(port);
				break;
			case '+':
				(port==0) ? startInflationAll() : startInflation(port);
				if(_inf==true && _vac==true) pressure = getPressure();
				break;
			case '-':
				(port==0) ? startVacuumAll() : startVacuum(port);
				if(_inf==true && _vac==true) pressure = getPressure();
				break;
			case 'P': //inflate2x
				(port==0) ? startInflationAll2x() : startInflation2x(port); //these will not do anything unless _inf=true & _vac=false.
				if(_inf==true && _vac==false) pressure = getPressure();
				break;
			case 'N': //vacuum2x
				(port==0) ? startVacuumAll2x() : startVacuum2x(port); //these will not do anything unless _vac=true & _inf=false.
				if(_inf==false && _vac==true) pressure = getPressure();
				break;
			case '^':
				(port==0) ? startReleaseAll() : startRelease(port);
				pressure = getPressure();	
				break;
			
			case 'o':
				if(port=='o') powerOFF(); //power off when 'oo' is sent.
				break;
			case 'r':
				redLED(port); //if port>0 led=on, if port=0 led=off.
				break;
			case 'b':
				blueLED(port);
				break;
			case '?':
				stopActionAll();
				(port==0) ? openAllPorts() : openPort(port);
				pressure = getPressure();
				break;
			return pressure;
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

//API2: functions to perform actions involving more than one element.
	void FlowIO::startInflation(uint8_t portNumber){ 
		stopAction(portNumber);
	  	if(_inf==true){
		  	openInletValve(); 	//(If we don't have an inlet valve, it is ok b/c then this will act on pin "0")
		  	openPort(portNumber);
		  	startPump(1);
		  }
	  	
	}
	void FlowIO::startInflation2x(uint8_t portNumber){ //Only applicable for a DualInflation system.
	  	stopAction(portNumber);
	  	if(_inf==true && _vac==false){
	  		openPort(portNumber);
	  		startPump(1);
	  		startPump(2);
	  	}
	}
	void FlowIO::startVacuum(uint8_t portNumber){ 
	//NOTE: In this case it actually matters which is the Outlet and which the release valve.
		stopAction(portNumber); 
		if(_vac==true){
			openPort(portNumber);
			startPump(2);
			if(_inf==true){ //if InflationVacuum system, the release valve is connected to the vacuum pump,
		  		openReleaseValve(); //thus it must be opened. Otherwise it must be kept closed.
		  	}
		}
	}
	void FlowIO::startVacuum2x(uint8_t portNumber){ //Only applicable for a DualVacuum system.
		stopAction(portNumber);
		if(_inf==false && _vac==true){
			openPort(portNumber);  
			startPump(1);
			startPump(2);
		}
	}
	void FlowIO::startRelease(uint8_t portNumber){
		stopAction(portNumber);
		openReleaseValve();
		openPort(portNumber);  
	}
	void FlowIO::stopAction(uint8_t portNumber){
		closeInletValve();
		closeReleaseValve();
		closePort(portNumber);
	  	stopPump(1);
		stopPump(2);
	}	
	void FlowIO::startInflationAll(){
		stopActionAll();
		if(_inf==true){
			openAllPorts();
			openInletValve();
			startPump(1);
		}	
	}
	void FlowIO::startInflationAll2x(){ //Only applicable for a DualInflation system.
		stopActionAll();
		if(_inf==true && _vac==false){
			openAllPorts();
			startPump(1);
		  	startPump(2);
		}	
	}
	void FlowIO::startVacuumAll(){
		//NOTE: For this one it matters which is the relief and which the outlet valve.
		stopActionAll();
		if(_vac==true){
			openAllPorts();			
			startPump(2);
			if(_inf==false){ //if DualVacuum system, the release vave is NOT connected to the pump
				closeReleaseValve(); 	//thus it must be closed.
			}
			else if(_inf==true){ //if inflationVacuum system, the release valve is the outlet valve
				openReleaseValve();		//connected to the vacuum pump, and it must be opened. 
			}
		}
	}
	void FlowIO::startVacuumAll2x(){ //Applicable only for a DualVacuum system.
		stopActionAll();
		if(_inf==false && _vac==true){
			openAllPorts();
			startPump(1);
			startPump(2);
		}
	}
	void FlowIO::startReleaseAll(){
		stopActionAll();
		openAllPorts();
		openReleaseValve();
	}
	void FlowIO::stopActionAll(){
		closeInletValve();
		closeReleaseValve(); //making sure that the Exhaust is closed.
		closeAllPorts();
		stopPump(1);
		stopPump(2);
	}


//API: functions to perform time-dependent actions (these are based on the above, and not on the driver directly) 
//------NOTE: THESE FUNCTIONS ARE BLOCKING EXECUTION (they include delay() elements.)
  	void FlowIO::inflateT(uint8_t portNumber, int millisec){ //this is a blocking function
		if(_inf==true){
			if(millisec > 0){
				startInflation(portNumber);
				delay(millisec);
				stopAction(portNumber);
			}
		}
	}
	void FlowIO::releaseT(uint8_t portNumber, int millisec){ //this is a blocking function
		if(millisec > 0){
			startRelease(portNumber);
			delay(millisec);
			stopAction(portNumber);
		}
	}
	void FlowIO::vacuumT(uint8_t portNumber, int millisec){ //this is a blocking function
		if(_vac==true){
			if(millisec > 0){
				startVacuum(portNumber);
				delay(millisec);
				stopAction(portNumber);
			}
		}	
	}
	void FlowIO::inflateAllT(int millisec){ //inflates all valves simultaneously.
		if(_inf==true){
			if(millisec>0){		
				startInflationAll();
				delay(millisec);
				stopActionAll();			
			}
		}
	}
	void FlowIO::releaseAllT(int millisec){
		if(millisec>0){
			startReleaseAll();
			delay(millisec);
			stopActionAll();
		}
	}
	void FlowIO::vacuumAllT(int millisec){
		if(_vac==true){
			if(millisec>0){
				startVacuumAll();
				delay(millisec);
				stopActionAll();
			}
		}
	}

	//API: Presure Dependent Methods (BLOCKING)
	
	//I will require pressure unit to be specified explicity, because otherwise user may forget what units
	//they had set and accidentally start working in different units, causing something to blow up. Moreover,
	//when units are specified explicity, this enables one to have different inflation operations happening in
	//different units, say if someone has inflatables that are rated at different pressure systems.
	void FlowIO::inflateP(uint8_t portNumber, float pMax, unit pUnit){ //the value of pMax is of unit as specified in the units function.
		if(getPressure(pUnit)<pMax) startInflation(portNumber);
		while(getPressure(pUnit)<pMax){
			delay(1);
		}
		stopAction(portNumber);		
	}