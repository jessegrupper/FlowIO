/* If you are manually sending serial commands from the serial monitor, you need to 
 * ensure that "No Line Ending" is chosen in the Serial Monitor window.  */

#define VBAT_PIN          A7            //A7 for nrf52832, A6 for nrf52840
#define VBAT_RESOLUTION   0.73242188F   //For 12bit @ 3V reference, resolution is 3000mV/4096steps = 0.732421875mV/step.
#define VBAT_DIVIDER_COMP 1.403F        // this is 4.2/3
#define REAL_VBAT_RESOLUTION (VBAT_RESOLUTION * VBAT_DIVIDER_COMP) //this is in units of millivolts/step.

#define _BAUDRATE 115200 //9600

float readVBAT() {
  analogReference(AR_INTERNAL_3_0);   // Set the analog reference to 3.0V (default = 3.6V) b/c battery voltage may drop to <3.6V and then default will become inaccurate.
  analogReadResolution(12);   // Set the resolution to 12-bit (0..4095). Can be 8, 10, 12 or 14
  int raw;
  float vbatMv; //battery voltage in millivolt
  raw = analogRead(VBAT_PIN);   // Get the raw 12-bit, 0..3000mV ADC value
  vbatMv = raw * REAL_VBAT_RESOLUTION; //we multiply the read value times the millivols per step.
  return vbatMv; 
}

uint8_t getBatteryPercentage(){ //this computation is based on LiPo chemistry. 
  float mv = readVBAT();
  if(mv<3300)    return 0; 
  if(mv<3600) {
    mv -= 3300;
    return mv/30;
  } 
  mv -= 3600;
  return 10 + (mv * 0.15F );  // thats mv/6.66666666
}

#include <FlowIO.h>
FlowIO flowio;
int pressureReportedAt=0;
int batteryPercentage=10;
float pressure;
int timePrevious=millis();

char val;

//poweroff timer
int offTimerStart = millis();
bool remaining1minute=false;
bool remaining2minute=false;

void setup() {
  Serial.begin(_BAUDRATE);
  //flowio = FlowIO(INFLATION_SERIES);
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
  flowio.command('!', 0xFF); //stopAction(0b11111111);
  //flowio.sendCommand('?','?'); //send a request for pressure. Do I want to add this?
}

void loop() { 
  if(Serial.available() >= 2){ 
    char actionChar = Serial.read(); //read the 1st char
    char portsChar  = Serial.read();         //read the 2nd char
    pressure = flowio.command(actionChar, portsChar);
    if(actionChar=='?'){
      Serial.print("Pres:");
      Serial.println(pressure);
    }
    Serial.flush();
  }
  //Send the battery level every half second.
  if(millis()-timePrevious > 500){
    Serial.print("Batt:");
    Serial.println(getBatteryPercentage());
    timePrevious = millis();
  }
  autoPowerOff(30); //argument is the number of minutes of inactivity.
}

void autoPowerOff(int minutes){
  int remainingtime=minutes-(millis() - offTimerStart)/60000;
  if(millis() - offTimerStart > minutes*60000){
    offTimerStart = millis(); //reset it. This is useful in case of a hardware failure and power is not cut off properly.
    Serial.println("Powering off");
    flowio.powerOFF();
  }
  else if(remainingtime==1 && remaining1minute==false){
    Serial.println("Powering off in 1 minute");  
    remaining1minute=true;
  }
  else if(remainingtime==2 && remaining2minute==false){
    Serial.println("Powering off in 2 minutes");  
    remaining2minute=true;
  }
}
