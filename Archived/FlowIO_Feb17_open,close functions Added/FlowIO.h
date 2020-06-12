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
#define MAXPORTS 6

enum unit : uint8_t{
	PSI, HPA, ATM
};
enum configuration : uint8_t{ //pneumatic configuration mode
	ALL_PURPOSE, INFLATIONPP, VACUUMPP
};

class FlowIO{
private:
	int _portsInUse; //this can be smaller than the number of actual valves in the system.
	uint8_t _pumpPins[2]={3,4};
	uint8_t _portValvePins[MAXPORTS]={28,29,15,7,11,0}; //The last item is irrelevant in the case of mode 1. I don't need to change this array.	
	uint8_t _inletValvePin;
	uint8_t _releaseValvePin;
	bool _inf; //inflation support
	bool _vac; //vacuum support
	bool _i2cInitialized=false; //becomes true when the activateSensor() function has been executed.
	void _setConfigMode(configuration mode); 	
	//void _setConfigMode(uint8_t mode);
	uint8_t _addr = 0x18;
    TwoWire *_i2c;   
    uint8_t statusByte;
    uint8_t _getStatusByte(void);
    uint32_t _getRawPressure(void);
    float   _getPressureHPA();
    float   _getPressurePSI();
    float   _getPressureATM();
    unit _pressureUnit=PSI;
public:	
	//constructors
	FlowIO();//default where mode=GENERAL_PURPOSE, ports=5
	FlowIO(uint8_t portsInUse);
	FlowIO(uint8_t portsInUse, configuration mode);	

	//DRIVER: functions to control individual elements
	bool 	activateSensor();
	void 	setPressureUnit(unit pUnit); //default is PSI.
	float 	getPressure();
    float   getPressure(unit pUnit);	

   	void startPump(uint8_t pumpNumber);
	void stopPump(uint8_t pumpNumber);
	void openInletValve();
	void closeInletValve();
	void openOutletValve();
	void closeOutletValve();
	void openReleaseValve();
	void closeReleaseValve();
	void openPort(uint8_t portNumber);
	void closePort(uint8_t portNumber);
	void openAllPorts();
	void closeAllPorts();

	void blueLED(bool power);
	void redLED(bool power);
	void powerOFF();

	//NEW DRIVER
  	void setPorts(uint8_t portsByte); //Opens the ports with 1. Closes the ports with 0.
  	void openPorts(uint8_t ports); //Opens the ports with 1. Leaves the ports with ports with 0 unchanged.
  	void closePorts(uint8_t ports); //Closes ports with 1. Leaves the ports with 0 unchanged.
  
	//API1 Two character based control
	float sendCommand(uint8_t action, uint8_t ports);

	//API2: functions to perform actions involving more than one element.
	void startInflation(uint8_t portNumber);
	void startInflation2x(uint8_t portNumber);
	void startVacuum(uint8_t portNumber);	
	void startVacuum2x(uint8_t portNumber);	
	void startRelease(uint8_t portNumber);	
	void stopAction(uint8_t portNumber);
	void startInflationAll();
	void startInflationAll2x();
	void startVacuumAll();
	void startVacuumAll2x();
	void startReleaseAll();
	void stopActionAll(); //serves as both stopInflatingAll and stopDeflatingAll

	//API: Time Dependent Methods. (Blocking)
	void inflateT(uint8_t portNumber, int millisec); //blocking function
	void inflate2xT(uint8_t portNumber, int millisec); //blocking function
	void vacuumT(uint8_t portNumber, int millisec); //blocking function
	void vacuum2xT(uint8_t portNumber, int millisec); //blocking function
	void releaseT(uint8_t portNumber, int millisec); //blocking function
	void inflateAllT(int millisec);
	void inflateAll2xT(int millisec);
	void vacuumAllT(int millisec);
	void vacuumAll2xT(int millisec);
	void releaseAllT(int millisec);

	//API: Presure Dependent Methods
	void inflateP(uint8_t portNumber, float pMax, unit pUnit);

};

#endif