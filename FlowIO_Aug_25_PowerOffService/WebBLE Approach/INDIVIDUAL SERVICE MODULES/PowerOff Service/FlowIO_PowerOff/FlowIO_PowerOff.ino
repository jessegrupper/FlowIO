/*This example implements a service with one characteristic for powering off the FlowIO.
 * It periodically the hardwareState variable for any hardware state changes, and if there
 * has been a change, then it resets the off timer. The off timer also resets during connect
 * and disconnect events. 
*/

#include <bluefruit.h>
#include <FlowIO.h>

#define DEVICE_NAME "FlowIO_pwr"// Device Name: Maximum 30 bytes

FlowIO flowio;

BLEService powerOffService;
BLECharacteristic chrPowerOffTimer;

void setup() {
  Serial.begin(115200);
  flowio = FlowIO(GENERAL);
  Bluefruit.begin();
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  createPowerOffService(); //This is defined in the file "powerService.ino"
  startAdvertising();
}

void startAdvertising(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.addService(powerOffService); //advertise service uuid

  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
  
  Bluefruit.Advertising.start(0);
}

void loop() {
  checkIfTimeToPowerOffEvery(5000); //Even though we are calling this function every time
  //we go through this loop, the contents of the function actually execute only every 5 sec.
  //There is a clever algorightm in it that checks if 5 sec has passed.
}

void connect_callback(uint16_t conn_handle){ // callback invoked when central connects  
  resetOffTimer(); //reset the timer to 5 minutes.
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){
  restartOffTimer(); //we RESTART the timer to whatever value the user had previsouly chosen in the GUI.
}
