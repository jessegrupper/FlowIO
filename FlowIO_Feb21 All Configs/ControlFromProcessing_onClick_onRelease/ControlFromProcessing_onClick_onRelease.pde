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

int myColor = color(255);
int c1,c2;
float n,n1;
boolean p1 = false;
boolean p2 = false;
boolean p3 = false;
boolean p4 = false;
boolean p5 = false;
byte ports;

void setup() {
  size(1200,1200);
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 115200);
    
  noStroke();
  cp5 = new ControlP5(this);
   
  //create a new font
  PFont p = createFont("Verdana",15);  
  ControlFont myFont = new ControlFont(p);
  cp5.setFont(myFont);
  
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
  cp5.addSlider("PSI")  
     .setRange(15, 20)
     .setPosition(900, 120)
     .setSize(250, 40)
     ; 
  
  cp5.addButton("-1").setPosition(200,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('p');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('p');}});
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
  
  
  cp5.addButton("POWER OFF").setPosition(0,0).setSize(150,40)
     .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('f'); myPort.write('f');}});
  cp5.addButton("Blue LED").setPosition(160,0).setSize(150,40)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('b'); myPort.write('1');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('b'); myPort.write(0x00);}});
  cp5.addButton("Red LED17").setPosition(320,0).setSize(150,40)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('r'); myPort.write('1');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('r'); myPort.write(0x00);}});
  cp5.addButton("Pressure").setPosition(480,0).setSize(150,40);
  cp5.addButton("Value").setPosition(750,0).setSize(150,40);
  cp5.addButton("Batt").setPosition(1100,0).setSize(50,40);

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
void draw() {
  ports=0x00;
  if(p5) ports ^= 0x01;
  if(p4) ports ^= 0x02;
  if(p3) ports ^= 0x04;
  if(p2) ports ^= 0x08;
  if(p1) ports ^= 0x10;

  
  getBatteryPercentage();
  background(myColor);
  myColor = lerpColor(c1,c2,n);  
  
  textSize(20);
  text("Batt:", 1050,30); 
  text("Inflate:", 50,150); //inflation label
  text("maxP:", 900,110); //inflation label
  text("Vacuum:", 50,300); 
  text("Release:", 50,450);
  text("Sense:", 50,600);
  text("setPorts: ", 50,825);
  text("Actions: ", 50,920);
}

void stopActionAll(){
    myPort.write('!');
    myPort.write('0');
    delay(10); //helps reduce the number of serial messages being sent.
}
//The pressure readng happens only once per state. Each time a state character is sent a new pressure reading is made. Thus, all the control as far as delay
//must be set in the processing code and not in the embedded code. There must be no delays inside the state machine on the SOC ever.
void getBatteryPercentage(){
  while(myPort.available() > 0){
    String str = myPort.readStringUntil('\n');
    if(str != null){
      if(str.length() > 7){
        if(str.substring(0,5).equals("Batt:")){ //https://processing.org/reference/String.html
          String percentage = str.substring(5,7); //returns positions 5 and 6 (starting position is 0).
          cp5.getController("Batt").setCaptionLabel(percentage);
        }
      }
    }
  }
}
