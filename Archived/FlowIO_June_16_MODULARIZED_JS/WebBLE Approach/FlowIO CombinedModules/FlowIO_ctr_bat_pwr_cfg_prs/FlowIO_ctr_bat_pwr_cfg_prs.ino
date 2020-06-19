#include <bluefruit.h>
#include <FlowIO.h>

#define DEVICE_NAME "FlowIO_ctr_bat_pwr_cfg_prs"// Device Name: Maximum 30 bytes

FlowIO flowio;

BLEService controlService;
BLECharacteristic chrCommand;
BLECharacteristic chrHardwareState;
uint8_t cmd[2]; //holds the 2-byte command

BLEService batteryService;
BLECharacteristic chrBattPercentage;

BLEService powerOffService;
BLECharacteristic chrPowerOffNow;
BLECharacteristic chrPowerOffWhenInactiveFor;
BLECharacteristic chrPowerOffRemainingTime;

BLEService configService;
BLECharacteristic chrConfig;

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
  Serial.begin(9600); //remove the serial printouts.
  flowio = FlowIO(GENERAL); //This must be done BEFORE any Bluefruit commands!
  if(flowio.activateSensor()==false) flowio.redLED(HIGH);
  analogReference(AR_INTERNAL_3_0);   // Set the analog reference to 3.0V (default = 3.6V) b/c battery voltage may drop to <3.6V and then default will become inaccurate.
  analogReadResolution(12);   // Set the resolution to 12-bit (0..4095). Can be 8, 10, 12 or 14
  Bluefruit.autoConnLed(true);   // Setup the BLE LED to be enabled on CONNECT
  //All Bluefruit.config***() function must be called before Bluefruit.begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX); // Config the peripheral connection with maximum bandwidth
  Bluefruit.configUuid128Count(15); //by default this is 10, and we have more than 10 services & characteristics on FlowIO
  Bluefruit.begin();
  Bluefruit.setTxPower(0); //Accepted values: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  createControlService(); //this is defined in "controlService.ino"
  createBatteryService(); //this is defined in "batteryService.ino"
  createPowerOffService(); //This is defined in the file "powerOffService.ino"
  createConfigService(); //This is defined in "configService.ino"
  createPressureService();

  startAdvertising();   // Set up and start advertising
}

void startAdvertising(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
    //Choose which services we want to advertise. No need to advertise custom services
  Bluefruit.Advertising.addService(batteryService);   //advertise service uuid
  Bluefruit.Advertising.addService(powerOffService);

  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode

  Bluefruit.Advertising.start(0);
}

void loop() {
    //Serial.println(flowio.getCurrentHardwareState());
    updateBatteryLevelEvery(5000);
    checkIfTimeToPowerOffEvery(5000);
    waitForEvent();  // Request CPU to enter low-power mode until an event/interrupt occurs
}

void connect_callback(uint16_t conn_handle){
  resetOffTimer();
  //stop everything on connect
  Serial.println("Connected");
  cmd[0] = '!';
  cmd[1] = 0b00011111;
  flowio.command(cmd[0],cmd[1]);
  //You can replace the above 3 lines with: flowio.stopAction(0x00011111);
  chrCommand.write(cmd,2);
  chrHardwareState.notify16(flowio.getHardwareState());
  chrBattPercentage.notify8(getBatteryPercentage());
  chrConfig.notify8(flowio.getConfig());

  float num = flowio.getPressure(PSI);
  chrPressureValue.notify(&num,sizeof(num));
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){
  resetOffTimer();
  //stop everything on disconnect
  cmd[0] = '!';
  cmd[1] = 0b00011111;
  flowio.command(cmd[0],cmd[1]);
  //You can replace the above 3 lines with flowio.stopAction(0x00011111);
  //but leave them like this for clarity of how the protocol can be used.
}
