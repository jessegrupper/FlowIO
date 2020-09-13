#include <bluefruit.h>
#include <FlowIO.h>

#define DEVICE_NAME "FlowIO_gpio"// Device Name: Maximum 30 bytes

FlowIO flowio;

BLEService gpioService;
BLECharacteristic chrPin02; //analog pin

void setup() {
  Serial.begin(115200);
  flowio = FlowIO(GENERAL);
  Bluefruit.begin();
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.setTxPower(4);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  createGpioService();
  startAdvertising();
}

void startAdvertising(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.addService(gpioService); //advertise service uuic

  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode

  Bluefruit.Advertising.start(0);
}

void loop() {
  notifyInputValueChange_Pin02_ifdV(5); //this function sends a notification if: notifymode==true && dV>5.
}

void connect_callback(uint16_t conn_handle){
  
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){
  //stop everything on disconnect
}
