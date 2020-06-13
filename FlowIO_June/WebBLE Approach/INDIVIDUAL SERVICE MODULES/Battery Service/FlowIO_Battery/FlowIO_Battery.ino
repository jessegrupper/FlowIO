/*Check battery percentage every 5 seconds, and if changed send the battery level as a notification to the connected central.
 *During any time, the central still has the ability to request the battery level.
 *https://www.youtube.com/watch?v=oCMOYS71NIU)
 *
 */

#include <bluefruit.h>
#define DEVICE_NAME "FlowIO_bat"// Device Name: Maximum 30 bytes

BLEService batteryService;
BLECharacteristic chrBattPercentage;

void setup(){
  Serial.begin(9600);
  analogReference(AR_INTERNAL_3_0);   // Set the analog reference to 3.0V (default = 3.6V) b/c battery voltage may drop to <3.6V and then default will become inaccurate.
  analogReadResolution(12);   // Set the resolution to 12-bit (0..4095). Can be 8, 10, 12 or 14
  Bluefruit.autoConnLed(true);   // Setup the BLE LED to be enabled on CONNECT
  //All Bluefruit.config***() function must be called before Bluefruit.begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX); // Config the peripheral connection with maximum bandwidth
  Bluefruit.configUuid128Count(15); //by default this is 10, and we have more than 10 services & characteristics on FlowIO
  Bluefruit.begin();
  Bluefruit.setTxPower(0); // Set max power. Accepted values: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.Periph.setConnectCallback(connect_callback);

  createBatteryService(); //this is defined in "batteryService.ino"
  startAdvertising();   // Set up and start advertising
}

void loop(){
  updateBatteryLevelEvery(5000);
  waitForEvent();  // Request CPU to enter low-power mode until an event/interrupt occurs
}

void startAdvertising(void){
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);   // Advertising packet
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.restartOnDisconnect(true); //Restart advertising on disconnect.
  Bluefruit.ScanResponse.addName();   // Secondary Scan Response packet (optional)
  Bluefruit.Advertising.addService(batteryService);   //advertise service uuid

  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode

  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void connect_callback(uint16_t conn_handle){ // callback invoked when central connects
  chrBattPercentage.notify8(getBatteryPercentage());
}
