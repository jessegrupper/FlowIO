/*The BLE parts of this code are taken from the example "bleuart" in 
 File>>Examples>>Adafruit Bluefruit nRF52 Libraries >> Peripheral >> bleuart
 
 *Controlling Pneumatics Driver Board via BLE. 
 *This firmware controls one valve at a time, or all valves simultaneously.  
 *There is NO algorithm in this code to prevent overinflation and bursting.
 *
 *NOTE: For serial communication there is always 1 extra character due to the Enter key!!!
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

#include <bluefruit.h>
#include <FlowIO.h>

//#define charsInBuffer 5 //this is the length of data packets
#define MSG_SIZE 2

//Action states. Enums hold consecutive integers starting from 0 by default. 
enum State : uint8_t{ //if we don't set the type it would default to 'int'.
  STOP, INFLATE, INFLATE2X, RELEASE, VACUUM, VACUUM2X, SENSE, POWEROFF, RED, BLUE
};

BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
//There is a BLE batter service, defined by Adafruit, which we can just use.
//All service libraries: C:\Users\Ali\AppData\Local\Arduino15\packages\adafruit\hardware\nrf52\0.14.0\libraries\Bluefruit52Lib\src\services
BLEBas  blebas;  // battery service

bool ledstate=1;
float pressure=0.0;

FlowIO flowio;
static char state = STOP; //stores the action state name
char actionChar = '!'; //holds first character of message. Set default to 'stop'.
char valveChar = '0'; //holds second character of message. Set default to 'all valves'
char portNumberChar = '0';
int portNumber = 0; //this holds the integer version of portNumber, which is what we want.
//char printBuff[50]; //buffer to be used with: sprintf(printBuff, "The time is %i", time); Serial.print(printBuff);

//poweroff timer
int offTimerStart = millis();
//int offTimer = millis() - offTimerStart
//if(millis() - offTimerStart > 5000) powerOff();

//battery percentage
int bPer = 0; 
#define VBAT_PIN          (A7)
#define VBAT_MV_PER_LSB   (0.73242188F)   // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096
#define VBAT_DIVIDER      (0.71275837F)   // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)        // Compensation factor for the VBAT divider

//function to read raw battery value from A7
int readVBAT(void) {
  int raw;
  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);
  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14
  // Let the ADC settle
  delay(2);
  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(VBAT_PIN);
  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(10);
  return raw;
}

//function to convert raw value to pecentage value
uint8_t mvToPercent(float mvolts) {
    uint8_t battery_level;
    if (mvolts >= 3000)        battery_level = 100;
    else if (mvolts > 2900)    battery_level = 100 - ((3000 - mvolts) * 58) / 100;
    else if (mvolts > 2740)    battery_level = 42 - ((2900 - mvolts) * 24) / 160;
    else if (mvolts > 2440)    battery_level = 18 - ((2740 - mvolts) * 12) / 300;
    else if (mvolts > 2100)    battery_level = 6 - ((2440 - mvolts) * 6) / 340;
    else                       battery_level = 0;    
    return battery_level;
}

void setup(){ 
  Serial.begin(115200);
  while (!Serial) delay(10);   // for nrf52840 with native usb
  flowio = FlowIO();
//  while(flowio.activateSensor()==false){
//    flowio.redLED(HIGH);
//    delay(50);
//  }
  flowio.redLED(LOW);
  //flowio.setPressureUnit(PSI);
  
  readVBAT(); // Get a single ADC sample and throw it away  
  Bluefruit.autoConnLed(true);   // Setup the BLE LED to be enabled on CONNECT
  //All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX); // Config the peripheral connection with maximum bandwidth 
  Bluefruit.begin();
  Bluefruit.setTxPower(4); // Set max power. Accepted values: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setName("FlowIO Platform");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  bledis.setManufacturer("Ali");
  bledis.setModel("OTA Control 1");
  bledis.begin();

  bleuart.begin();   // Configure and Start BLE Uart Service

  // Start BLE Battery Service
  blebas.begin();    
  blebas.write(100);

  startAdv();   // Set up and start advertising
}

void startAdv(void){
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);   // Advertising packet
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);   // Include bleuart 128-bit uuid
  Bluefruit.ScanResponse.addName();   // Secondary Scan Response packet (optional)
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop(){ 
  int vbat_raw = readVBAT();   // Get a raw ADC reading

  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_raw * VBAT_MV_PER_LSB);
  // checks to see if the battery percentage has changed, if it has it notifies the central
  if (bPer != vbat_per) {
      Serial.println("Changed");
      blebas.notify(vbat_per);
      bPer = vbat_per;
  }
  
  // This is if we want to send commands from the chip via BLE.
  while (Serial.available()){
    //delay(2);  // Delay to wait for enough input, since we have a limited transmission buffer
    uint8_t buf[20];
    int count = Serial.readBytes(buf, sizeof(buf));
    bleuart.write(buf,count);
  }
  //This is for reading data that has been sent to the chip.
  if(bleuart.available() >= MSG_SIZE){ 
    resetOffTimer(); 
    actionChar = (char) bleuart.read(); //read the 1st char
    portNumberChar  = bleuart.read();         //read the 2nd char
    portNumber = portNumberChar - '0'; //convert the char to an int. '0'=48, '1'=49, etc.  
    Serial.println(portNumber);
    bleuart.flush();        
  }

  switch(state){ 
    case STOP:
      (portNumber==0) ? flowio.stopActionAll() : flowio.stopAction(portNumber); 
      //pressure = flowio.getPressure();
      nextState(actionChar);
      break;
    case INFLATE:
      (portNumber==0) ? flowio.startInflationAll() : flowio.startInflation(portNumber);
      //pressure = flowio.getPressure();
      nextState(actionChar);
      break;
    case INFLATE2X:
      (portNumber==0) ? flowio.startInflationAll2x() : flowio.startInflation2x(portNumber);
      //pressure = flowio.getPressure();
      nextState(actionChar);
      break; 
    case RELEASE:
      (portNumber==0) ? flowio.startReleaseAll() : flowio.startRelease(portNumber);
      //pressure = flowio.getPressure();
      nextState(actionChar);
      break; 
    case VACUUM:
      (portNumber==0) ? flowio.startVacuumAll() : flowio.startVacuum(portNumber);
      //pressure = flowio.getPressure();
      nextState(actionChar);
      break;
    case VACUUM2X:
      (portNumber==0) ? flowio.startVacuumAll2x() : flowio.startVacuum2x(portNumber);
      //pressure = flowio.getPressure();
      nextState(actionChar);
      break;   
    case POWEROFF:
      flowio.powerOFF();
      nextState(actionChar);
      break;
    case RED: //toggle red led
      ledstate=!ledstate;
      flowio.redLED(ledstate);
      nextState(actionChar);
      break;
    case SENSE:
      flowio.openPort(portNumber);
      //pressure = flowio.getPressure();
      nextState(actionChar);
      break;
  }
  waitForEvent();  // Request CPU to enter low-power mode until an event/interrupt occurs
  powerOffIfInactiveFor(10000);

  //If the state has not changed or if the device has not been connected, then power it off after 1 minute. Alternatively, whenever there is 
  //a user event taking place, I can reset the poweroff timer. The following are condidered events:
      //data is sent to the chip
      //connect event occurs
      //disconnect event occurs.
}

void nextState(char actionChar){
  if      (actionChar=='!') state = STOP;
  else if (actionChar=='+') state = INFLATE;
  else if (actionChar=='P') state = INFLATE2X;
  else if (actionChar=='^') state = RELEASE;
  else if (actionChar=='-') state = VACUUM;
  else if (actionChar=='N') state = VACUUM2X;
  
  else if (actionChar=='o') state = POWEROFF;
  else if (actionChar=='r') state = RED;
  //else if (actionChar=='?') state = SENSE;
}
void powerOffIfInactiveFor(int inactiveTime){
  if(millis() - offTimerStart > inactiveTime){
    offTimerStart = millis(); //reset it. This is useful in case of a hardware failure and power is not cut off properly.
    Serial.println("Powering off");
  }
}
void resetOffTimer(){
  offTimerStart = millis();
}

void connect_callback(uint16_t conn_handle){ // callback invoked when central connects
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
  
  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
  Serial.print("Connected to ");
  Serial.println(central_name);
  resetOffTimer();


  //gets raw battery value
  int vbat_raw = readVBAT();
  uint8_t vbat_per = mvToPercent(vbat_raw * VBAT_MV_PER_LSB);
  //Notifies central of battery percentage
  blebas.notify(vbat_per);
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){ //// callback invoked when connection dropped
  (void) conn_handle; //conn_handle connection where this event happens
  (void) reason; //reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
  Serial.println("\n Disconnected");
  Serial.println("Closing all valves and pumps");
  flowio.stopActionAll();
  bleuart.flush(); //clear the buffer if connection is dropped, so no other commands are executed after disconnect.
  resetOffTimer();
}
