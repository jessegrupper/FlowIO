#include <bluefruit.h>
#define MSG_SIZE 2

char actionChar = '!'; //holds first character of message. Set default to 'stop'.
char portNumberChar = '0';
int batteryLevelReportedAt = 0; //we will set this millis() when we send the battery level.
bool connectCallbackFinishedExecution = false; //this flag is to allow for the callback to finish execution before the void loop starts executing.

BLEClientBas  clientBatteryService; 
BLEClientDis  clientDeviceInfoService; 
BLEClientUart clientUartService;

void setup(){
  Serial.begin(115200);
  while (!Serial) delay(10);   // for nrf52840 with native usb
  Serial.println("Bluefruit52 Central BLEUART Example \n");
  
  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setName("Bluefruit52 Central");

  clientBatteryService.begin();  // Configure Battery client
  clientDeviceInfoService.begin();  // Configure DIS client

  // Init BLE Central Uart Serivce
  clientUartService.begin();
  clientUartService.setRxCallback(bleuart_rx_callback);

  Bluefruit.setConnLedInterval(250);  // Increase Blink rate to different from PrPh advertising mode

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Don't use active scan
   * - Start(timeout) with timeout = 0 will scan forever (until connected)
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);                   // // 0 = Don't stop scanning after n seconds
}

void scan_callback(ble_gap_evt_adv_report_t* report){ //invoked when scanner picks up advertising data
  if ( Bluefruit.Scanner.checkReportForService(report, clientUartService) ){   // Check if advertising contain BleUart service
    Serial.print("BLE UART service detected. Connecting ... ");
    Bluefruit.Central.connect(report);    // Connect to device with bleuart service in advertising
  }else{ 
    // For Softdevice v6: after received a report, scanner will be paused. We need to call Scanner resume() to continue scanning
    Bluefruit.Scanner.resume();
  }
}

void connect_callback(uint16_t conn_handle){ //invoked when connection is established
  connectCallbackFinishedExecution = false;
  Serial.print("Connected \n Dicovering Device Information ... ");
  if(clientDeviceInfoService.discover(conn_handle)){
    Serial.println("Found it");
    char buffer[32+1];
    
    memset(buffer, 0, sizeof(buffer));
    if(clientDeviceInfoService.getManufacturer(buffer, sizeof(buffer)) ){
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    memset(buffer, 0, sizeof(buffer));
    if(clientDeviceInfoService.getModel(buffer, sizeof(buffer)) ){
      Serial.print("Model: ");
      Serial.println(buffer);
    }
  }

  if (clientBatteryService.discover(conn_handle) ){
    Serial.print("Battery level (%): ");
    Serial.println(clientBatteryService.read());
  } 

  if (clientUartService.discover(conn_handle) ){
    Serial.println("Enable TXD's notify");
    clientUartService.enableTXD();
    Serial.println("Ready to receive from peripheral");
  }else{
    Bluefruit.disconnect(conn_handle);    // disconnect since we couldn't find bleuart service
  }  
  connectCallbackFinishedExecution = true;
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason){ //invoked when connection is lost
  (void) conn_handle;
  (void) reason;
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

void bleuart_rx_callback(BLEClientUart& uart_svc){ //invoked when uart data is received
  Serial.print("[RX]: ");
  while (uart_svc.available() ) {
    Serial.print((char)uart_svc.read());
  }
  Serial.println();
}

void loop(){
  if(Bluefruit.Central.connected() && connectCallbackFinishedExecution==true){    
      if(Serial.available() >= MSG_SIZE){ //I should specify the action character to be one of the known characters only using a case block.
        actionChar = Serial.read();
        portNumberChar  = Serial.read();
        transmit(actionChar, portNumberChar);
        Serial.flush(); //On the Feather, this DOES clear the input buffer! (even though on regular arduino it does not)
      }
      
      //Refresh and send over serial the battery level every 100ms. (Refresh rate of FlowIO is 1000ms, much slower).
      if(millis() - batteryLevelReportedAt > 100){
        Serial.print("Batt:");
        Serial.println(clientBatteryService.read());
        batteryLevelReportedAt = millis();
      }
  }

      
  waitForEvent();
}

void transmit(char actionChar, char portNumberChar){
  if(Bluefruit.Central.connected()){    
    if(clientUartService.discovered()){
      clientUartService.write(actionChar);
      clientUartService.write(portNumberChar);
    }
  }
}
