#include <bluefruit.h>
#include <FlowIO.h>

#define DEVICE_NAME "FlowIO_ctr"// Device Name: Maximum 30 bytes

FlowIO flowio;

BLEService controlService;
  BLECharacteristic chrCommand;
  BLECharacteristic chrHardwareState;
  uint8_t cmd[3]; //holds the command. cmd[0]=action, cmd[1]=port, cmd[2]=pwm.

void setup() {
  Serial.begin(9600);
  flowio = FlowIO(GENERAL); //This must be done BEFORE any Bluefruit commands!
  //flowio.activateSensor();
  Bluefruit.autoConnLed(true);   // Setup the BLE LED to be enabled on CONNECT. Do I need this?
  //All Bluefruit.config***() function must be called before Bluefruit.begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX); // Config the peripheral connection with maximum bandwidth. Do I need this?
  Bluefruit.configUuid128Count(15); //by default this is 10, and we have more than 10 services & characteristics on FlowIO
  Bluefruit.begin();
  Bluefruit.setTxPower(0); //Accepted values: -40, -30, -20, -16, -12, -8, -4, 0, 4  
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  createControlService(); //this is defined in "controlService.ino"
  startAdvertising();
  
}

void startAdvertising(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.addService(controlService); //advertise service uuic

  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
  
  Bluefruit.Advertising.start(0);
}

void loop() {
    //Serial.println(flowio.getCurrentHardwareState());
}

void connect_callback(uint16_t conn_handle){
  //stop everything on connect
  Serial.println("Connected");
  cmd[0] = '!';
  cmd[1] = 0b00011111;
  cmd[3] = 0; //this is irrelevant for '!'. But we are sending the full cmd array anyways to BLE.
  flowio.command(cmd[0],cmd[1]);
  chrCommand.write(cmd,3); //We are writing the full 3-byte cmd array. 
  chrHardwareState.notify16(flowio.getHardwareState());
}
void disconnect_callback(uint16_t conn_handle, uint8_t reason){
  //stop everything on disconnect
  cmd[0] = '!';
  cmd[1] = 0b00011111;
  flowio.command(cmd[0],cmd[1]);
}
