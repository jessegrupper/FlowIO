#include <FlowIO.h>
FlowIO flowio;

uint8_t cmd[2]; //holds the 2-byte command

void setup() {
  Serial.begin(115200);
  flowio = FlowIO(GENERAL); //This must be done BEFORE any Bluefruit commands!
  flowio.command('!',0b00011111);
  flowio.activateSensor();
 // flowio.startInflation(0b00000011);
  Serial.println("Status Byte:");
  Serial.println(flowio.getHardwareState(),BIN);
  Serial.println("Individual Components:");
  Serial.print(flowio.getHardwareStateOf(PORT1));
  Serial.print(flowio.getHardwareStateOf(PORT2));
  Serial.print(flowio.getHardwareStateOf(PORT3));
  Serial.print(flowio.getHardwareStateOf(PORT4));
  Serial.print(flowio.getHardwareStateOf(PORT5));
  Serial.print(flowio.getHardwareStateOf(PUMP1));
  Serial.print(flowio.getHardwareStateOf(PUMP2));
  Serial.print(flowio.getHardwareStateOf(INLET));
  Serial.print(flowio.getHardwareStateOf(OUTLET));
  Serial.print(flowio.getHardwareStateOf(LEDRED));
  Serial.print(flowio.getHardwareStateOf(LEDBLUE));
  Serial.println(flowio.getHardwareStateOf(SENSOR));
}

void loop() {
//  flowio.startInflation(0x01); //port1 (left side).
//  delay(1000);
//  flowio.stopAction(0xFF);
//  delay(1000);
//  flowio.startInflation(0x01, 150); 
//  delay(1000);
//  flowio.stopAction(0xFF);
//  delay(1000);
}
