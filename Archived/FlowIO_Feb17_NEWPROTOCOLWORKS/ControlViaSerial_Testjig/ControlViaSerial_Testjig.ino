#include <FlowIO.h>
FlowIO flowio;

char val;
void setup() {
  Serial.begin(115200);
}

void loop() {
//  if(Serial.available() > 0){
//    val = Serial.read();
//    //flowio.startInflation(val);
//    flowio.sendCommand('+', val);
//    Serial.println(val,BIN);
//    //flowio.setPorts(val);
//  }

  if(Serial.available() >= 2){ 
    char actionChar = Serial.read(); //read the 1st char
    char portsChar  = Serial.read();         //read the 2nd char
    flowio.sendCommand(actionChar, portsChar);
    Serial.flush();
    while(Serial.available()>0) Serial.read();
  }
}
