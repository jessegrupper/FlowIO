#include <FlowIO.h>
FlowIO flowio;

char val;
void setup() {
  Serial.begin(115200);
}

void loop() {
  if(Serial.available() > 0){
    val = Serial.read();
    //flowio.startInflation(val);
    flowio.sendCommand('+', val);
    Serial.println(val,BIN);
    //flowio.setPorts(val);
  }
}
