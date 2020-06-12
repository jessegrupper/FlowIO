

/*Controlling Pneumatics Driver Board via Serial. 
 *This firmware controls one valve at a time, or all valves simultaneously.  
 *There is NO algorithm in this code to prevent overinflation and bursting.
 *
 *NOTE: For serial communication there is always 1 extra character for the Enter key!!!
 *  Thus if we are sending 2-character messages, the actual length would be 3 characters!!!
 *  However, for the case of BLE communication, the number of characters is just 2. 
 *  
 * The communication protocol uses 2-character strings where the first character is the action 
 * to be performed, and the second character is the Port # for which that action is intended. 
 * If port number is 0, that refers to 'all valves'. Refer to the documentation for more details.
 *    "+3" -- inflate port 3.
 *    "P3" -- inflate port 3 at 2x speed.
 *    "!3" -- stop any action on port 3.
 *    "^0" -- release all ports (deflate).
 * 
 * TUTORIAL ON STATE MACHINES: https://hackingmajenkoblog.wordpress.com/2016/02/01/the-finite-state-machine/
 */

 //TODO: I want to put my communication protocol implementation into the library.
 //TODO: Write documentation about the FlowIO library. 

#include <FlowIO.h>

#define MSG_SIZE 2

//Action states. Enums hold consecutive integers starting from 0 by default. 
enum State : uint8_t{ //if we don't set the type it would default to 'int'.
  STOP, INFLATE, INFLATE2X, RELEASE, VACUUM, VACUUM2X, SENSE, POWEROFF, RED, BLUE
};


bool pinstate=1;

FlowIO flowio;
static char state = STOP; //stores the action state name
char actionChar = '!'; //holds first character of message. Set default to 'stop'.
char valveChar = '0'; //holds second character of message. Set default to 'all valves'
char portNumberChar = '0';
uint8_t portNumber = 0; //this holds the integer version of portNumber, which is what we want.
//char printBuff[50]; //buffer to be used with: sprintf(printBuff, "The time is %i", time); Serial.print(printBuff);

float pressure;

void setup(){  
  Serial.begin(115200);
  while (!Serial) delay(10);   // for nrf52840 with native usb 
  flowio = FlowIO();
  flowio.setPressureUnit(PSI);
  if (flowio.activateSensor()==false){
    while(1){
        flowio.redLED(HIGH);
        delay(100);
        flowio.redLED(LOW);
        delay(100);
    }
  }  
  //Turn on the Blue and Red LED
  flowio.blueLED(pinstate);
  flowio.redLED(HIGH);
}

void loop(){
  if(Serial.available() >= MSG_SIZE+1){ //I tried setting it to MSG_SIZE only and I get a bad behavior.
    actionChar = (char) Serial.read();
    portNumberChar  = Serial.read(); //we need to convert this to int type.
    portNumber = portNumberChar - '0'; //converts the char to an int. '0'=48, '1'=49, etc.
    Serial.flush(); //On the Feather, this DOES clear the input buffer! (even though on regular arduino it does not)
  }

  //TODO: this value must be sent from processing
  float pMax = 18.0;
  //I can move the state machine to the library. I don't want to do this, because I can have complicated state machines
  //implemented on the device, not just this basic one. But I would like to be able to save the
  //communication protocol to the library somehow, without saving the state machine.
//  flowio.stateMachine(actionChar,portNumber){
//    switch(state){
//      case STOP:
//        
//    }
//  }
  
  switch(state){ 
    case STOP:
      (portNumber==0) ? flowio.stopActionAll() : flowio.stopAction(portNumber); 
      nextState(actionChar); //This sets the state variable to one of the available states. It goes there on next iteration.
      break;
    case INFLATE:
      //I need to get the pressure at the port at which I am inflating. Thus, I must read the
      //pressure AFTER I have opend the port. 
      (portNumber==0) ? flowio.openAllPorts() : flowio.openPort(portNumber);
      if(flowio.getPressure()<pMax){
        (portNumber==0) ? flowio.startInflationAll() : flowio.startInflation(portNumber);
      }
      pressure = flowio.getPressure();
      Serial.println(pressure);
      nextState(actionChar);
      break;
    case INFLATE2X:
      (portNumber==0) ? flowio.startInflationAll2x() : flowio.startInflation2x(portNumber);
      pressure = flowio.getPressure();
      Serial.println(pressure);
      nextState(actionChar);
      break; 
    case RELEASE:
      (portNumber==0) ? flowio.startReleaseAll() : flowio.startRelease(portNumber);
      pressure = flowio.getPressure();
      Serial.println(pressure);
      nextState(actionChar);
      break; 
    case VACUUM:
      (portNumber==0) ? flowio.startVacuumAll() : flowio.startVacuum(portNumber);
      pressure = flowio.getPressure();
      Serial.println(pressure);
      nextState(actionChar);
      break;
    case VACUUM2X:
      (portNumber==0) ? flowio.startVacuumAll2x() : flowio.startVacuum2x(portNumber);
      pressure = flowio.getPressure();
      Serial.println(pressure);
      nextState(actionChar);
      break;   
    case POWEROFF:
      flowio.powerOFF();
      nextState(actionChar); //in case power off doesn't occur.
      break;
    case BLUE: //toggle blue led 
      pinstate=!pinstate;
      flowio.blueLED(pinstate);
      nextState(actionChar);
      break;
    case RED: //toggle red led
      pinstate=!pinstate;
      flowio.redLED(pinstate);
      nextState(actionChar);
      break;
    case SENSE:
      flowio.openPort(portNumber);
      pressure = flowio.getPressure();
      Serial.println(pressure);
      nextState(actionChar);
      break; 
  }
  waitForEvent();  // Request CPU to enter low-power mode until an event/interrupt occurs
}

void nextState(char& actionChar){
  if      (actionChar=='!') state = STOP;
  else if (actionChar=='+') state = INFLATE;
  else if (actionChar=='P') state = INFLATE2X;
  else if (actionChar=='^') state = RELEASE;
  else if (actionChar=='-') state = VACUUM;
  else if (actionChar=='N') state = VACUUM2X;
  
  else if (actionChar=='o') state = POWEROFF;
  else if (actionChar=='b') state = BLUE;
  else if (actionChar=='r') state = RED;

  else if (actionChar=='?') state = SENSE;
}
