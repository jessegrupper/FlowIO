//TODO: In the main loop I should check if inflation==true & pressure limit exceeded, then stop Inflation.
//And I will have similar checks for all the other actions and all ports. If the pressure is exceeded, I
//don't even want to start the action in the first place.But this requires to check the pressure at the
//port before starting the action. It will cause a slight delay however.

#include <bluefruit.h>
#include <FlowIO.h>

#define DEVICE_NAME "FlowIO_prs"// Device Name: Maximum 30 bytes

FlowIO flowio;


BLEService pressureService;
BLECharacteristic chrPressureValue;
BLECharacteristic chrPressureRequest;
BLECharacteristic chrMaxPressureLimits;
BLECharacteristic chrMinPressureLimits;

//Define 2 arrays to hold our pressure limits, which will update whenever the limits are changed
//from the GUI. Based on these values, we will decide whether to allow an action to take place.
float minLimits[5];
float maxLimits[5];


void setup() {
  Serial.begin(9600);
  flowio = FlowIO(GENERAL);
  if(flowio.activateSensor()==false) flowio.redLED(HIGH);
  Bluefruit.begin();
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.setTxPower(4);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  createPressureService();
  startAdvertising();
}

void startAdvertising(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.addService(pressureService); //advertise service uuic

  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode

  Bluefruit.Advertising.start(0);
}

void loop() {
  notifyPressure_IfActive_IfDeltaP(0.005); //Sends a pressure notification if device is running and if pressure has changed
      //by more than the PSI value specified in the argument. This checks the status byte to determine if device is running.
}

void connect_callback(uint16_t conn_handle){
  float num = flowio.getPressure(PSI);
  chrPressureValue.notify(&num,sizeof(num));
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){
  //stop everything on disconnect
}
