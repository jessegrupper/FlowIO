/* FlowIO.h - a library for controlling the pneumatic circuit
	(I need to add this circuit board and the Tiny8SSR to my website)
   Created by Ali Shtarbanov, Sept. 2019
*/

#ifndef FlowIO_h
#define FlowIO_h

#include "Arduino.h"
#include "Wire.h"

#define MPRLS_POWERED        (0b01000000) ///< Status SPI powered bit
#define MPRLS_BUSY           (0b00100000) ///< Status busy bit
#define MPRLS_INTEGRITY_FAIL (0b00000100) ///< Status bit for integrity fail
#define MPRLS_SATURATION     (0b00000001) ///< Status bit for math saturation

#define blueLEDpin 19
#define redLEDpin 17
#define powerOFFpin 16

enum Unit : uint8_t{
	PSI, HPA, ATM
};
enum Configuration : uint8_t{ //pneumatic configuration mode
	GENERAL, INFLATION_SERIES, VACUUM_SERIES, INFLATION_PARALLEL, VACUUM_PARALLEL
};

class FlowIO{
private:
	uint8_t _pumpPins[2]={3,4};
	uint8_t _portValvePins[5]={28,29,15,7,11}; //The last item is irrelevant in the case of mode 1. I don't need to change this array.	
	uint8_t _inletValvePin=30;
	uint8_t _outletValvePin=5;
	Configuration _config;
	bool _i2cInitialized=false; //becomes true when the activateSensor() function has been executed.
	void _setConfig(Configuration config); 	
	uint8_t _addr = 0x18;
    TwoWire *_i2c;   
    uint8_t statusByte;
    uint8_t _getStatusByte(void);
    uint32_t _getRawPressure(void);
    float   _getPressureHPA();
    float   _getPressurePSI();
    float   _getPressureATM();
    Unit _pressureUnit=PSI;
public:	
	//constructors
	FlowIO();
	FlowIO(Configuration config);	

	//DRIVER: functions to control individual elements
	bool 	activateSensor();
	void 	setPressureUnit(Unit pUnit); //default is PSI.
	float 	getPressure();
    float   getPressure(Unit pUnit);	

   	void startPump(uint8_t pumpNumber);
	void stopPump(uint8_t pumpNumber);
	void openInletValve();
	void closeInletValve();
	void openOutletValve();
	void closeOutletValve();

	void blueLED(bool power);
	void redLED(bool power);
	void powerOFF();

  	void setPorts(uint8_t ports); //Opens the ports with 1. Closes the ports with 0.
  	void openPorts(uint8_t ports); //Opens the ports with 1. Leaves the ports with ports with 0 unchanged.
  	void closePorts(uint8_t ports); //Closes ports with 1. Leaves the ports with 0 unchanged.
  
	//API1 Two character based control
	float command(uint8_t action, uint8_t ports);

	//API2: functions to perform actions involving more than one element.
	void startInflation(uint8_t ports);
	void startInflationHalfCapacity(uint8_t ports);
	void startVacuum(uint8_t ports);	
	void startVacuumHalfCapacity(uint8_t ports);	
	void startRelease(uint8_t ports);	
	void stopAction(uint8_t ports);

	//API: Time Dependent Methods. (Blocking)
	void inflateT(uint8_t ports, int millisec); //blocking function
	void inflate2xT(uint8_t ports, int millisec); //blocking function
	void vacuumT(uint8_t ports, int millisec); //blocking function
	void vacuum2xT(uint8_t ports, int millisec); //blocking function
	void releaseT(uint8_t ports, int millisec); //blocking function

	//API: Presure Dependent Methods
	void inflateP(uint8_t ports, float pMax, Unit pUnit);

};

#endif