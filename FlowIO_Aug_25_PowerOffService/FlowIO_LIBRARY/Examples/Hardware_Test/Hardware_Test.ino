/*This is the hardware testng firmware used for ensuring that the device works as expected.
 * Open your serial monitor to read the instructions for how to run each test, and the 
 * outcomes and results. You need to have access to the DFU button for this test. 
 * Follow the instructions on the Serial Monitor.
 */
#include <FlowIO.h>
#define dfuPin 20


FlowIO flowio;
int mode = 0;
int prevMode = -1; //makeing this different so we enter the state machine on first start.

bool buttonState = 0;         // current state of the button
bool prevButtonState = 0;     // previous state of the button
float p0 = 0;
float pinf=0;
float pvac=0;

void setup(){
  Serial.begin(115200);
  flowio = FlowIO(GENERAL);
  pinMode(dfuPin,INPUT_PULLUP); //DFUwea
  Serial.println("\n### --FlowIO Testing Firmware Initialized - ###\n");
  flowio.blueLED(HIGH);
  flowio.stopAction(0xFF);
}

void loop() {
  if(mode != prevMode){ //Only execute this code if the mode has changed.
    switch(mode){
      case 0: //we come here when we first start the system
        flowio.blueLED(0);
        sensorTest();
        Serial.println("------\nPress DFU button to begin Test 1: Valve Click Test");
        Serial.println(" - each of the 7 valve will turn on and off sequentially. You should count 14 clicks.");
        break;
      case 1:
        flowio.blueLED(1);
        delay(500); //delay to allow enough time to release button
        valveClickTest();
        Serial.println("Test 1 Complete. If successfull, move on, otherwise power off");
        Serial.println("------\nPress DFU button to start Test 2: Inflation Pump Test");
        Serial.println(" - the inflation pump will run for 3sec. Internal pressure should increase. Air goes out through the other pump");
        break;
      case 2:
        flowio.blueLED(1);
        delay(500); //delay to allow enough time to release button
        inflationPumpTest();
        Serial.println("Test 2 Complete. If successfull, move on, otherwise power off");
        Serial.println("------\nPress DFU button to start Test 3: Vacuum Pump Test)");
        Serial.println(" - the vacuum pump will run for 3sec. Internal pressure should decrease. Air comes in through the other pump.");
        break;
      case 3:
        flowio.blueLED(1);
        delay(500); //delay to allow enough time to release button
        vacuumPumpTest();
        Serial.println("Test 3 Complete. If successfull, move on, otherwise power off");
        Serial.println("------\nPress DFU button to start Test 4: Air Leaks");
        Serial.println(" - ");
        break;
      case 4:
        flowio.blueLED(1);
        delay(500); //delay to allow enough time to release button
        sensorTest();
        Serial.println("Test 4 Complete. If successfull, move on, otherwise power off");
        Serial.println("------\nPress DFU button to start Test 5: Air Leaks");
        Serial.println(" - The pressure should increase until.");
        break;
    }
    prevMode = mode;
  }
  buttonState = digitalRead(dfuPin);
  if(buttonState != prevButtonState){ //if buttonstate has changed.
    if(buttonState == LOW)  //and if it is now pressed.
      mode += 1;
      if(mode>4) mode=0;
      delay(50); //debounce
  }
  prevButtonState = buttonState;  
}


void valveClickTest(){
  Serial.print("Valve Click Test in Progress....");
  Serial.print("in..");
  flowio.openInletValve();
  delay(500);
  flowio.closeInletValve();
  delay(1000);
  Serial.print("out..");
  flowio.openOutletValve();
  delay(500);
  flowio.closeOutletValve();
  delay(1000);
  Serial.print("p1..");
  flowio.setPorts(0b00000001);
  delay(500);
  flowio.setPorts(0b00000000);
  delay(1000);
  Serial.print("p2..");
  flowio.setPorts(0b00000010);
  delay(500);
  flowio.setPorts(0b00000000);
  delay(1000);
  Serial.print("p3..");
  flowio.setPorts(0b00000100);
  delay(500);
  flowio.setPorts(0b00000000);
  delay(1000);
  Serial.print("p4..");
  flowio.setPorts(0b00001000);
  delay(500);
  flowio.setPorts(0b00000000);
  delay(1000);
  Serial.println("p5.");
  flowio.setPorts(0b00010000);
  delay(500);
  flowio.setPorts(0b00000000);
  delay(1000);  
}
void sensorTest(){
  //To get the correct pressure reading, we will open and close two ports first
  flowio.openPorts(0b00000011);
  delay(100);
  flowio.stopAction(0xFF);
  Serial.println("Sensor Test in Progress....");
  flowio.activateSensor();
  if(flowio.readError()){
    Serial.print("Detecting Sensor......FAILED. #$%@# Error ");
    Serial.println(flowio.readError());
    flowio.redLED(1);
  }
  else{
    Serial.print("Detecting Sensor......Success! :) ");
    Serial.print("P = ");
    p0=flowio.getPressure(PSI);
    Serial.println(p0);
  }
}
void inflationPumpTest(){
  Serial.print("Inflation Pump Test in Progress....");
  flowio.openInletValve();
  flowio.openOutletValve();
  flowio.startPump(1);
  delay(1500);
  Serial.print("P = ");
  pinf=flowio.getPressure(PSI);
  Serial.print(pinf);
  if(pinf<=(p0+0.4)) Serial.println("...FAILED"); //pressure should be at least 0.4psi higher.
  else Serial.println("...Success! :)");
  delay(1500);
  flowio.stopPump(1);
  flowio.closeOutletValve();
  flowio.closeInletValve();
}
void vacuumPumpTest(){
  Serial.print("Vacuum Pump Test in Progress....");
  flowio.openInletValve();
  flowio.openOutletValve();
  flowio.startPump(2);
  delay(1500);
  Serial.print("P = ");
  pvac=flowio.getPressure(PSI);
  Serial.print(pvac);
  if(pvac>=(p0-0.4)) Serial.println("...FAILED"); //pressure should be at least 0.4psi lower.
  else Serial.println("...Success! :)");
  delay(1500);
  flowio.stopPump(2);
  flowio.closeOutletValve();
  flowio.closeInletValve();
}
