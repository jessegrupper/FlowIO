#include <FlowIO.h>
FlowIO flowio;

uint8_t cmd[2]; //holds the 2-byte command

void setup() {
  Serial.begin(115200);
  flowio = FlowIO(GENERAL); //This must be done BEFORE any Bluefruit commands!
  cmd[0] = '!';
  cmd[1] = 0b00011111;
  flowio.command(cmd[0],cmd[1]);
  Serial.println(flowio.getHardwareState());
}

void loop() {
  flowio.startInflation(0x01); //port1 (left side).
  delay(1000);
  flowio.stopAction(0xFF);
  delay(1000);
  flowio.startInflation(0x01, 150); 
  delay(1000);
  flowio.stopAction(0xFF);
  delay(1000);
}
