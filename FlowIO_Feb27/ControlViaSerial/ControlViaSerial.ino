/* If you are manually sending serial commands from the serial monitor, you need to 
 * ensure that "No Line Ending" is chosen in the Serial Monitor window.  */

#include <FlowIO.h>
FlowIO flowio;
int pressureReportedAt=0;
float pressure;

char val;
void setup() {
  Serial.begin(115200);
  //flowio = FlowIO(VACUUM_SERIES);
  flowio = FlowIO(GENERAL);
  flowio.setPressureUnit(PSI);
  if (flowio.activateSensor()==false){
    while(1){
        flowio.redLED(HIGH);
        delay(100);
        flowio.redLED(LOW);
        delay(100);
    }
  } 
  flowio.blueLED(HIGH); //this can also be achieves using sendCommand()
  flowio.redLED(HIGH); //but we want all actions possible from sendCommand to be doable from other functions.
  flowio.sendCommand('!', 0xFF); //stopAction(0b11111111);
}

void loop() {
  if(Serial.available() >= 2){ 
    char actionChar = Serial.read(); //read the 1st char
    char portsChar  = Serial.read();         //read the 2nd char
    pressure = flowio.sendCommand(actionChar, portsChar);
    if(actionChar=='?'){
      Serial.print("Pres:");
      Serial.println(pressure);
    }
    Serial.flush();
  }
//  if(millis() - pressureReportedAt > 70){
//    pressure = flowio.getPressure();
//    Serial.print("Pres:");
//    Serial.println(pressure);
//    pressureReportedAt=millis();
//  }
}
