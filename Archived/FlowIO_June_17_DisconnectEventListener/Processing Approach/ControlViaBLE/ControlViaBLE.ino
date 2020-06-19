/*The BLE parts of this code are taken from the example "bleuart" in 
 File>>Examples>>Adafruit Bluefruit nRF52 Libraries >> Peripheral >> bleuart
 * 
 * We create a manual service for the pressure, and use the same UUID for it as the blood pressure monitor service's UUID.
 * And to that service, we attach one characteristic. 
 * It is through this custom-defined service & characteristic that we are able to send the pressure information to the
 * intermediate board. 
 * 
 * TODO: Have the pressure be reported only when requested, not broadcasted at some intervals. The interval chosen 
 * may be too big in some instances, and in others it may not be necesary to broadcast pressure info at all. We 
 * want to get the pressure only upon request.
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
  //Serial.println(vbatMv);
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
  //Serial.println(10 + (mv * 0.15F ));
  return 10 + (mv * 0.15F );  // thats mv/6.66666666
  
}

#define MSG_SIZE 2

//All included services are in: C:\Users\Ali\AppData\Local\Arduino15\packages\adafruit\hardware\nrf52\0.14.0\libraries\Bluefruit52Lib\src\services
BLEDis  deviceInfoService; 
BLEUart uartService;
BLEBas  batteryService;

//Here we define our custom service. We use the BloodPressure service UUID, but we don't fully implement that protocol.
BLEService pressureService = BLEService(UUID16_SVC_BLOOD_PRESSURE);
BLECharacteristic pressureCharacteristic = BLECharacteristic(UUID16_CHR_BLOOD_PRESSURE_MEASUREMENT);
void setupPressureService(){
  pressureService.begin();
  pressureCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  pressureCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  pressureCharacteristic.setFixedLen(4); //float is 32bits
  pressureCharacteristic.begin();
}

float pressure;

FlowIO flowio;

char actionChar = '!'; //holds first character of message. Set default to 'stop'.
char portsChar = 0b11111111;

//poweroff timer
int offTimerStart = millis();
bool remaining1minute=false;
bool remaining2minute=false;

void setup(){ 
  Serial.begin(115200);

  while (!Serial) delay(10);   // for nrf52840 with native usb
  //flowio = FlowIO(GENERAL);
  flowio = FlowIO(INFLATION_SERIES);
  //flowio = FlowIO(VACUUM_SERIES);
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
  flowio.setPressureUnit(PSI);
  while(flowio.activateSensor()==false){
    flowio.redLED(HIGH);  delay(100);  flowio.redLED(LOW);  delay(100);
  }
  flowio.command('!', 0xFF); //stopAction(0b11111111);
  setupPressureService();
}

void loop(){ 
  //we want to get the battery percentage only once every 1 second.
  if(millis() - batteryLevelReportedAt >= 1000){
    batteryPercentage = getBatteryPercentage();
    if (batteryPercentage != batteryPercentage_prev) { //don't report if unchanged.
        batteryService.notify(batteryPercentage); //this is the part that actually sends the data over BLE
        batteryPercentage_prev = batteryPercentage;
    }  
    batteryLevelReportedAt = millis();
  }
//  if(millis() - pressureReportedAt >= 10){
//    pressureCharacteristic.write(&pressure,sizeof(pressure));
//    pressureReportedAt = millis();
//    //Serial.println(pressure);
//  }

  
  
  
  
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
    pressure = flowio.command(actionChar, portsChar);
      //The returned pressure value will be valid only if a pressure request is sent with the action character '?'. Otherwise,
      //the value of this variable will be 999.9 or 8888.0. 
  }
  
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
  flowio.stopAction(0b11111111);
  uartService.flush(); //clear the buffer if connection is dropped, so no other commands are executed after disconnect.
  resetInitialConditions();
}

void resetInitialConditions(){
  actionChar = '!';
  portsChar = 0b11111111;
  resetOffTimer();
}
void resetOffTimer(){
  offTimerStart = millis();
  remaining1minute=false;
  remaining2minute=false;
}
