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

//battery percentage (https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/nrf52-adc)
#define VBAT_PIN          A7            //A7 for nrf52832, A6 for nrf52840
#define VBAT_RESOLUTION   0.73242188F   //For 12bit @ 3V reference, resolution is 3000mV/4096steps = 0.732421875mV/step.
#define VBAT_DIVIDER_COMP 1.403F        // this is 4.2/3
#define REAL_VBAT_RESOLUTION (VBAT_RESOLUTION * VBAT_DIVIDER_COMP) //this is in units of millivolts/step.
uint8_t batteryPercentage = 0;
uint8_t batteryPercentage_prev = 0; 
int batteryLevelReportedAt = 0; //we will set this millis() when we send the battery level.

float readVBAT() {
  int raw;
  float vbatMv; //battery voltage in millivolt
  raw = analogRead(VBAT_PIN);   // Get the raw 12-bit, 0..3000mV ADC value
  vbatMv = raw * REAL_VBAT_RESOLUTION; //we multiply the read value times the millivols per step.
  return vbatMv; 
}

uint8_t getBatteryPercentage(){ //this computation is based on LiPo chemistry. 
  float mv = readVBAT();
  if(mv<3300)    return 0; 
  if(mv<3600) {
    mv -= 3300;
    return mv/30;
  } 
  mv -= 3600;
  return 10 + (mv * 0.15F );  // thats mv/6.66666666
}

#define MSG_SIZE 2

//All included services are in: C:\Users\Ali\AppData\Local\Arduino15\packages\adafruit\hardware\nrf52\0.14.0\libraries\Bluefruit52Lib\src\services
BLEDis  deviceInfoService; 
BLEUart uartService;
BLEBas  batteryService;

float pressure=0.0;

FlowIO flowio;

char actionChar = '!'; //holds first character of message. Set default to 'stop'.
char portsChar = 0b00000000;

//poweroff timer
int offTimerStart = millis();
bool remaining1minute=false;
bool remaining2minute=false;

void setup(){ 
  Serial.begin(115200);
 
  while (!Serial) delay(10);   // for nrf52840 with native usb
  flowio = FlowIO();
  flowio.redLED(LOW);

  analogReference(AR_INTERNAL_3_0);   // Set the analog reference to 3.0V (default = 3.6V) b/c battery voltage may drop to <3.6V and then default will become inaccurate.
  analogReadResolution(12);   // Set the resolution to 12-bit (0..4095). Can be 8, 10, 12 or 14
  Bluefruit.autoConnLed(true);   // Setup the BLE LED to be enabled on CONNECT
  //All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX); // Config the peripheral connection with maximum bandwidth 
  Bluefruit.begin();
  Bluefruit.setTxPower(4); // Set max power. Accepted values: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setName("FlowIO Platform");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  deviceInfoService.setManufacturer("Ali");
  deviceInfoService.setModel("OTA Control 1");
  deviceInfoService.begin();
  uartService.begin();    // Configure and Start BLE Uart Service
  batteryService.begin(); // Start BLE Battery Service
  startAdv();   // Set up and start advertising
//  while(flowio.activateSensor()==false){
//    flowio.redLED(HIGH);  delay(100);  flowio.redLED(LOW);  delay(100);
//  }
}

void loop(){ 
  //we want to get the battery percentage only once every 5 seconds and only if it has a change of more than 1%.
  if(millis() - batteryLevelReportedAt >= 1000){
    batteryPercentage = getBatteryPercentage();
    if (batteryPercentage != batteryPercentage_prev) { //don't report if unchanged.
        batteryService.notify(batteryPercentage);
        batteryPercentage_prev = batteryPercentage;
    }  
    batteryLevelReportedAt = millis();
  }
  
  
  // This is if we want to send commands from the chip via BLE.
  while (Serial.available()){
    //delay(2);  // Delay to wait for enough input, since we have a limited transmission buffer
    uint8_t buf[20];
    int count = Serial.readBytes(buf, sizeof(buf));
    uartService.write(buf,count);
  }
  //This is for reading data that has been sent to the chip.
  if(uartService.available() >= MSG_SIZE){ 
    resetOffTimer(); 
    actionChar = uartService.read(); //read the 1st char
    portsChar  = uartService.read();         //read the 2nd char
  }
  
  //############################################################################################
  pressure = flowio.sendCommand(actionChar, portsChar); //Do NOT put this inside the if block, b/c parameters may change not only from BLE.
  //############################################################################################

  //Serial.println(pressure);
  waitForEvent();  // Request CPU to enter low-power mode until an event/interrupt occurs
  autoPowerOff(3); //argument is the number of minutes of inactivity.
}

void autoPowerOff(int minutes){
  int remainingtime=minutes-(millis() - offTimerStart)/60000;
  if(millis() - offTimerStart > minutes*60000){
    offTimerStart = millis(); //reset it. This is useful in case of a hardware failure and power is not cut off properly.
    Serial.println("Powering off");
    flowio.powerOFF();
  }
  else if(remainingtime==1 && remaining1minute==false){
    Serial.println("Powering off in 1 minute");  
    remaining1minute=true;
  }
  else if(remainingtime==2 && remaining2minute==false){
    Serial.println("Powering off in 2 minutes");  
    remaining2minute=true;
  }
}

void startAdv(void){
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);   // Advertising packet
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(uartService);   // Include uartService 128-bit uuid
  Bluefruit.ScanResponse.addName();   // Secondary Scan Response packet (optional)
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle){ // callback invoked when central connects
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
  
  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
  Serial.print("Connected to ");
  Serial.println(central_name);
  resetInitialConditions();
  batteryService.notify(getBatteryPercentage());
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){ //// callback invoked when connection dropped
  (void) conn_handle; //conn_handle connection where this event happens
  (void) reason; //reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
  Serial.println("\n Disconnected");
  Serial.println("Closing all valves and pumps");
  flowio.stopActionAll();
  uartService.flush(); //clear the buffer if connection is dropped, so no other commands are executed after disconnect.
  resetInitialConditions();
}

void resetInitialConditions(){
  actionChar = '!';
  portsChar = 0b00000000;
  resetOffTimer();
}
void resetOffTimer(){
  offTimerStart = millis();
  remaining1minute=false;
  remaining2minute=false;
}
