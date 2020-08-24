#include "Arduino.h"
#include "FlowIO.h"

//PNEUMATICS API: functions to perform actions involving more than one element.
	void FlowIO::startInflation(uint8_t ports, uint8_t pwmValue){
		stopAction(ports);
		if(_config == GENERAL || _config == INFLATION_PARALLEL || _config == INFLATION_SERIES){
			if(ports<<3 != 0){ //if the ports are all 0, then don't do anything
				openInletValve(); 	//(If we don't have an inlet valve, it is ok b/c then this will act on pin "0")
				setPorts(ports);
				startPump(1, pwmValue);
				if(_config != GENERAL) startPump(2, pwmValue);
			}
		}
	}
	void FlowIO::startInflationHalfCapacity(uint8_t ports, uint8_t pwmValue){ //Only applicable for Inflation_Series and Inflation_Parallel configurations.
	  	stopAction(ports);
	  	if(_config == INFLATION_PARALLEL || _config == INFLATION_SERIES){
		  	if(ports<<3 != 0){
		  		openInletValve(); 
		  		setPorts(ports);
		  		startPump(1, pwmValue);
		  	}
	  	}
	}
	void FlowIO::startVacuum(uint8_t ports, uint8_t pwmValue){ 
		stopAction(ports); 
		if(_config == GENERAL || _config == VACUUM_PARALLEL || _config == VACUUM_SERIES){
			if(ports<<3 != 0){
				openOutletValve();
				setPorts(ports);
				startPump(2,pwmValue);
				if(_config != GENERAL) startPump(1, pwmValue);
			}
		}
	}
	void FlowIO::startVacuumHalfCapacity(uint8_t ports, uint8_t pwmValue){ //Only applicable for Vacuum_Series and Vacuum_Parallel configurations.
		stopAction(ports);
		if(_config == VACUUM_SERIES || _config == VACUUM_PARALLEL){
			if(ports<<3 != 0){	
				openOutletValve();
				setPorts(ports);  
				startPump(1, pwmValue);
			}
		}
	}
	void FlowIO::startRelease(uint8_t ports){
		stopAction(ports);		
		if(ports<<3 != 0){		
			setPorts(ports);
			if(_config == INFLATION_PARALLEL || _config == INFLATION_SERIES)
				openOutletValve();  //In the inflation configurations, the outlet valve is the pressure release valve as it's not connected.
			else if(_config == VACUUM_PARALLEL || _config == VACUUM_SERIES){
				openInletValve();	//In the vacuum configurations, the inlet valve is the pressure release valve as it's not connected.
			}
			else if(_config == GENERAL){ //Determine which valve to open based on the pressure value.
				if(getHardwareStateOf(SENSOR)==true){
					float p = _getPressurePSI();
					delay(10); //leave some time for the valve to open.
					float pAtmospheric = 14.53;
					if(p>=pAtmospheric) openOutletValve();
					else if(p<pAtmospheric) openInletValve();
					//TODO: Check if this also works OK for cases where we exceed the max pressure
					//and for cases where the sensor is activated but a pressure is not read fine.
				}
				//If the pressure sensor has not been activated, then open the outlet valve.
				else openOutletValve();
			}
		}
	}
	void FlowIO::stopAction(uint8_t ports){
		closeInletValve();
		closeOutletValve();
		//If we are stopping the action on port 5, we naturally would type stopAction(0b00000001), which means that
		//we want to CLOSE port 5. BUT we don't want to change any of the other 4 ports. Thus we use closePorts rather than setPorts().
		closePorts(ports); //this only closes the ports indicated with bit value 1.
	  	stopPump(1);
		stopPump(2);
	}	