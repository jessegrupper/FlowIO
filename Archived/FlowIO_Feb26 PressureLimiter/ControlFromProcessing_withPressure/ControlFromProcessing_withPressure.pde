//1. Install controlP5 by downloading the .zip file and estracting it into the /Documents/Processing/libraries folder.
//2. Download the Firmate library (cc.arduino) from Sketch >> Import Library >> Add library >> Arduino (Firmata)

import controlP5.*;
import processing.serial.*;

Serial myPort;
ControlP5 cp5;
Textlabel myLabel1;
Textlabel myLabel2;
Textlabel myLabel3;
Textlabel myLabel4;
Textlabel myLabel5;
Textlabel myLabel6;
Textlabel mylabelAll;
CheckBox checkbox;
Accordion accordion;

boolean p1 = false;
boolean p2 = false;
boolean p3 = false;
boolean p4 = false;
boolean p5 = false;
boolean blueLed=true;
boolean redLed=true;
byte ports;

float pressureValue=0;
boolean vacuuming = false; //these two variables will keep track of the state of the system. 
boolean inflating = false; 
float maxP; //if we have a slider/knob with the same string name, then the value of this variable will equal to the value of that slider.
float minP;

PImage pressureGaugeImage;
PImage batteryImage;

Knob minPressureKnob;
Knob maxPressureKnob;
Textlabel pressureValLabel;
Textlabel batteryPercentageLabel;

void setup() {
  size(1200,1000);

  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 115200);
    
  cp5 = new ControlP5(this);
   
  //create a new font
  PFont p = createFont("Verdana",15);  
  ControlFont myFont = new ControlFont(p);
  cp5.setFont(myFont);
  
  setupBarOnTop();
  setupInflationButtons();
  setupVacuumButtons();
  setupReleaseButtons();
  setupSenseButtons();
  setupPressureKnobs();
  setupMultiportButtons();

  

  
                      
  


     

}
void draw() {
  background(80, 79, 89);  
  image(pressureGaugeImage, 300,15);
  image(batteryImage, 550,15);
  ports=0x00;
  if(p5) ports ^= 0x01;
  if(p4) ports ^= 0x02;
  if(p3) ports ^= 0x04;
  if(p2) ports ^= 0x08;
  if(p1) ports ^= 0x10;
  
  //I can keep track of the state the system is in in Processing. Or I can have the system report to me the state it currently is operating in. 
  //For now, I would like to keep track in processing of just whether it is in a vacuuming or inflating state only. I don't care about configurations
  //or how many pumps are active or which ports are open. Just 2 boolean variables are all I need. 
  
  if(vacuuming==true && pressureValue <= minP){
     //send the stop command and set vacuuming to false.
     vacuuming=false;
     myPort.write('!');
     myPort.write(0xFF);
  }
  
  getBatteryAndPressure();
  
  textSize(20);
  text("Inflation:", 50,150); //inflation label
  text("Vacuum:", 50,300); 
  text("Release:", 50,450);
  text("Sense:", 50,600);
  text("setPorts: ", 50,825);
  text("Actions: ", 50,920);
  println(maxP);
}

/*###########################################################################################################################################
#####################################################---BEGIN FUNCTION DEFINITIONS---########################################################
*/

void stopActionAll(){
    myPort.write('!');
    myPort.write('0');
    delay(10); //helps reduce the number of serial messages being sent.
}

void getBatteryAndPressure(){
  while(myPort.available() > 0){
    String str = myPort.readStringUntil('\n');
    if(str != null){
      if(str.length() > 7){
        if(str.substring(0,5).equals("Batt:")){ //https://processing.org/reference/String.html
          String batteryStr = str.substring(5,7); //returns positions 5 and 6 (starting position is 0).
          batteryPercentageLabel.setText(batteryStr+ "%");
        }
        if(str.substring(0,5).equals("Pres:")){ 
          String pressureStr = str.substring(5,9); //returns positions 5 and 6 (starting position is 0).
          pressureValue = float(pressureStr);
          pressureValLabel.setText(pressureStr+ " psi");
        }
      }
    }
  }
}

void setupInflationButtons(){
  //I want the inflation to be maintained only while the button is BEING HELD pressed.
  //Once relased, the inflation should stop. If I want to inflate it more, i press it again. 
  //############################################################################################
  cp5.addButton("+1").setPosition(200,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('p');}}) //If I don't send the same command twice, then the FlowIO behaves in unintended way. Maybe something is bad with the FlowIO embedded code.
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('p');}});
  cp5.addButton("+2").setPosition(300,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('h');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('h');}});
  cp5.addButton("+3").setPosition(400,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('d');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('d');}});
  cp5.addButton("+4").setPosition(500,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('b');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('b');}});
  cp5.addButton("+5").setPosition(600,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('a');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('a');}});
    
    //.setColorBackground(color(0, 179, 255)) //default color
     //.setColorForeground(color(82, 204, 240)) //hover color
     //.setColorActive(color(0, 179, 255))  //when pressed
}
void setupVacuumButtons(){  
  cp5.addButton("-1").setPosition(200,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {
      //Now I have the functionality I wanted on port 1, where when I go below a certain pressure value, the system stops, But I want it automatically resume when the pressure comes back up again if I am still holding the button. I actuall don't want to do that.
      //if(pressureValue >= minP){
        vacuuming=true; myPort.write('-'); myPort.write('p');
      //}
    }})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('p'); vacuuming=false;}});
  cp5.addButton("-2").setPosition(300,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('h');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('h');}});
  cp5.addButton("-3").setPosition(400,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('d');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('d');}});
  cp5.addButton("-4").setPosition(500,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('b');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('b');}});
  cp5.addButton("-5").setPosition(600,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('a');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('a');}});
}

void setupReleaseButtons(){
  cp5.addButton("^1").setPosition(200,400).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('^'); myPort.write('p');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('p');}});
  cp5.addButton("^2").setPosition(300,400).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('^'); myPort.write('h');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('h');}});
  cp5.addButton("^3").setPosition(400,400).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('^'); myPort.write('d');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('d');}});
  cp5.addButton("^4").setPosition(500,400).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('^'); myPort.write('b');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('b');}});
  cp5.addButton("^5").setPosition(600,400).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('^'); myPort.write('a');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('a');}});
}

void setupSenseButtons(){
  cp5.addButton("?1").setPosition(200,550).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('?'); myPort.write('p');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('p');}});
  cp5.addButton("?2").setPosition(300,550).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('?'); myPort.write('h');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('h');}});
  cp5.addButton("?3").setPosition(400,550).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('?'); myPort.write('d');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('d');}});
  cp5.addButton("?4").setPosition(500,550).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('?'); myPort.write('b');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('b');}});
  cp5.addButton("?5").setPosition(600,550).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('?'); myPort.write('a');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('a');}});
}

void setupBarOnTop(){
  PImage[] pwrIcons = {loadImage("pwrOn.png"),loadImage("pwrHover.png"),loadImage("pwrOff.png")};
  cp5.addButton("PWR").setPosition(10,10).setImages(pwrIcons).updateSize()
     .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('f'); myPort.write('f');}});
  
  
  PImage[] blueLedIcons = {loadImage("blueLedOff.png"),loadImage("blueLedOn.png")};
  cp5.addToggle("blueLed").setPosition(160,15).setImages(blueLedIcons).updateSize()
     .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {
       if(blueLed){myPort.write('b'); myPort.write('1');}
       else{       myPort.write('b'); myPort.write(0x00);}}});
  
  PImage[] redLedIcons = {loadImage("redLedOff.png"),loadImage("redLedOn.png")};
  cp5.addToggle("redLed").setPosition(100,15).setImages(redLedIcons).updateSize()
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {
      if(redLed){myPort.write('r'); myPort.write('1');}
       else{       myPort.write('r'); myPort.write(0x00);}}});
 
 pressureGaugeImage = loadImage("pressure.png"); 
 pressureValLabel = cp5.addTextlabel("pressureValue")
                       .setPosition(350,15)
                       .setFont(createFont("Verdana",30))
                       .setText("?");
  
 batteryImage = loadImage("battery.png");
 batteryPercentageLabel = cp5.addTextlabel("batteryPercentage")
                       .setPosition(600,15)
                       .setFont(createFont("Verdana",30))
                       .setText("?");
}

void setupPressureKnobs(){
  maxPressureKnob = cp5.addKnob("maxP")
                       .setRange(15,20)
                       .setPosition(750,90)
                       .setRadius(50)
                       .setDragDirection(Knob.HORIZONTAL)
                       .setColorForeground(color(255,255,0))
                       .setColorBackground(color(0, 160, 100))
                       .setColorActive(color(255,255,0))
                       .setDefaultValue(15.0)
                       .setLabel("P max")
                       .setValue(18)
                       ;
  
  minPressureKnob = cp5.addKnob("minP")
                       .setRange(5,15)
                       .setPosition(750,220)
                       .setRadius(50)
                       .setDragDirection(Knob.HORIZONTAL)
                       .setColorForeground(color(255,255,0))
                       .setColorBackground(color(0, 160, 100))
                       .setColorActive(color(255,255,0))
                       .setDefaultValue(15.0)
                       .setLabel("P min")
                       .setValue(8)
                       ;
                       
  //cp5.addSlider("maxP")  
  //   .setRange(15, 20)
  //   .setPosition(800, 120)
  //   .setSize(250, 40)
  //   .setLabel("P max")
  //   ; 
  ////reposition the label for our slider.
  //cp5.getController("maxP").getValueLabel().align(ControlP5.RIGHT, ControlP5.BOTTOM_OUTSIDE).setPaddingX(0);
  //cp5.getController("maxP").getCaptionLabel().align(ControlP5.LEFT, ControlP5.BOTTOM_OUTSIDE).setPaddingX(0);                       
} 
void setupMultiportButtons(){
  cp5.addToggle("p1").setPosition(200,800).setSize(50,40);
  cp5.addToggle("p2").setPosition(270,800).setSize(50,40);
  cp5.addToggle("p3").setPosition(340,800).setSize(50,40);
  cp5.addToggle("p4").setPosition(410,800).setSize(50,40);
  cp5.addToggle("p5").setPosition(480,800).setSize(50,40);
     
  cp5.addButton("Inflate").setPosition(200,885).setSize(100,40)
     .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write(ports);}})
     .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write(ports);}});
  cp5.addButton("Vacuum").setPosition(320,885).setSize(100,40)
     .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write(ports);}})
     .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write(ports);}});
  cp5.addButton("Release").setPosition(440,885).setSize(100,40)
     .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('^'); myPort.write(ports);}})
     .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write(ports);}});
}
