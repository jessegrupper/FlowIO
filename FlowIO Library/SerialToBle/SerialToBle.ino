#include <bluefruit.h>
#define MSG_SIZE 2

char actionChar = '!'; //holds first character of message. Set default to 'stop'.
char portNumberChar = '0';

BLEClientBas  clientBas;  // battery client
BLEClientDis  clientDis;  // device information client
BLEClientUart bleclientuart; // bleuart client

void setup(){
  Serial.begin(115200);
  while (!Serial) delay(10);   // for nrf52840 with native usb
  Serial.println("Bluefruit52 Central BLEUART Example \n");
  
  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setName("Bluefruit52 Central");

  clientBas.begin();  // Configure Battery client
  clientDis.begin();  // Configure DIS client

  // Init BLE Central Uart Serivce
  bleclientuart.begin();
  bleclientuart.setRxCallback(bleuart_rx_callback);

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
  if ( Bluefruit.Scanner.checkReportForService(report, bleclientuart) ){   // Check if advertising contain BleUart service
    Serial.print("BLE UART service detected. Connecting ... ");
    Bluefruit.Central.connect(report);    // Connect to device with bleuart service in advertising
  }else{ 
    // For Softdevice v6: after received a report, scanner will be paused. We need to call Scanner resume() to continue scanning
    Bluefruit.Scanner.resume();
  }
}

void connect_callback(uint16_t conn_handle){ //invoked when connection is established
  Serial.print("Connected \n Dicovering Device Information ... ");
  if(clientDis.discover(conn_handle)){
    Serial.println("Found it");
    char buffer[32+1];
    
    // read and print out Manufacturer
    memset(buffer, 0, sizeof(buffer));
    if(clientDis.getManufacturer(buffer, sizeof(buffer)) ){
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    // read and print out Model Number
    memset(buffer, 0, sizeof(buffer));
    if(clientDis.getModel(buffer, sizeof(buffer)) ){
      Serial.print("Model: ");
      Serial.println(buffer);
    }
    
    Serial.println();
  }else{
    Serial.println("Found NONE");
  }

  Serial.print("Dicovering Battery Information ... ");
  if (clientBas.discover(conn_handle) ){
    Serial.println("Found it");
    Serial.print("Battery level: ");
    Serial.print(clientBas.read());
    Serial.println("%");
  }else  {
    Serial.println("Found NONE");
  }

  Serial.print("Discovering BLE Uart Service ... ");
  if (bleclientuart.discover(conn_handle) ){
    Serial.println("Found it");
    Serial.println("Enable TXD's notify");
    bleclientuart.enableTXD();
    Serial.println("Ready to receive from peripheral");
  }else{
    Serial.println("Found NONE");
    Bluefruit.disconnect(conn_handle);    // disconnect since we couldn't find bleuart service
  }  
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
  if(Bluefruit.Central.connected()){    
      //delay(2);
      if(Serial.available() >= MSG_SIZE+1){ //I should specify the action character to be one of the known characters only using a case block.
        actionChar = (char) Serial.read();
        portNumberChar  = (char) Serial.read();
        //if(actionChar=='+') transmit('+','1');
        //else if(actionChar=='-') transmit('-','1');
        //transmit('+','1');
        transmit(actionChar, portNumberChar);
        Serial.flush(); //On the Feather, this DOES clear the input buffer! (even though on regular arduino it does not)
      }
      //else transmit('!','0');


//      while (Serial.available()){
//        delay(2); 
//        uint8_t buf[20] = { 0 };
//        int count = Serial.readBytes(buf, sizeof(buf));
//        Serial.println(count);
//        bleclientuart.write(buf,count);
//        Serial.write(buf,count);
//      }
  }
  waitForEvent();
}

void transmit(char actionChar, char portNumberChar){
  if(Bluefruit.Central.connected()){    
    if(bleclientuart.discovered()){
      bleclientuart.write(actionChar);
      bleclientuart.write(portNumberChar);
    }
  }
}
