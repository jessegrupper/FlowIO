#include "Arduino.h"
#include "FlowIO.h"

/*
The valve that is not connected to anything is the one being use for pressure release:
The OUTLET valve is used for pressure release in the GENERAL, INFLATION_PARALLEL, INFLATION_SERIES configurations.
The INLET  valve is used for pressure release in the VACUUM_PARALLEL, VACUUM_SERIES configurations. 

TODO:
However, in the general configuration, both the INLET and OUTLET valves are connected to pumps. The pumps act like diodes, thus:
For release of high pressure, use OUTLET so air can exit freely through the vacuum pump.
For release of low pressure,  use INLET so air can enter freely through the inflation pump. 
When we press the release button, we can sense what is the pressure and then determine which valve to open. If the pressure
cannot be found, we just open the outlet valve.
*/

//CONSTRUCTOR: Assigns the pins as outputs. 
	//NOTE(1): The parameter variables have a scope limited to the consturctor. If we want to use some of those parameter
	//values that are passed by the user in a function other than the constructor, then we must save those values into
	//global variables (which we define with _underscore by convention) which have a scope not limited to the consturctor.
	//NOTE(2): There is a much more fundamental reason we need the these global member variables, and that has to do with
	//how they are stored in memory. When we create a member variable, a pointer is created that points to that variable. 
	
	FlowIO::FlowIO(){	//If no paramter is specified in the constructor, default to GENERAL.
		_setConfig(GENERAL); 
	}
	FlowIO::FlowIO(Configuration mode){
		_setConfig(mode);	
	}

	//Hardware Configuration Mode
	void FlowIO::_setConfig(Configuration config){
		_config = config; //set global variable to the parameter value.
		pinMode(_inletValvePin, OUTPUT);
		digitalWrite(_inletValvePin,LOW);
		pinMode(_outletValvePin, OUTPUT);
		digitalWrite(_outletValvePin,LOW);
		for(int i=0; i < 2; i++){
			pinMode(_pumpPins[i], OUTPUT);
			analogWrite(_pumpPins[i],0);
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

