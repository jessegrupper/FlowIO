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

int myColor = color(255);
int c1,c2;
float n,n1;

void setup() {
  size(1500,1200);
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
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('1'); myPort.write('+'); myPort.write('1');}}) //If I don't send the same command twice, then the FlowIO behaves in unintended way. Maybe something is bad with the FlowIO embedded code.
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('1'); myPort.write('!'); myPort.write('1');}});
  cp5.addButton("+2").setPosition(300,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('2'); myPort.write('+'); myPort.write('2');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('2'); myPort.write('!'); myPort.write('2');}});
  cp5.addButton("+3").setPosition(400,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('3');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('3');}});
  cp5.addButton("+4").setPosition(500,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('4');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('4');}});
  cp5.addButton("+5").setPosition(600,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('5');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('5');}});
  cp5.addButton("+ALL").setPosition(900,100).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('+'); myPort.write('0');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('0');}});
  cp5.addSlider("PSI")  
     .setRange(15, 20)
     .setPosition(1100, 120)
     .setSize(250, 40)
     ; 
  
  cp5.addButton("-1").setPosition(200,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('1');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('1');}});
  cp5.addButton("-2").setPosition(300,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('2');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('2');}});
  cp5.addButton("-3").setPosition(400,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('3');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('3');}});
  cp5.addButton("-4").setPosition(500,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('4');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('4');}});
  cp5.addButton("-5").setPosition(600,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('5');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('5');}});
  cp5.addButton("-ALL").setPosition(900,250).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('-'); myPort.write('0');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('0');}});
  
  cp5.addButton("^1").setPosition(200,400).setSize(80,80)
    .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('^'); myPort.write('1');}})
    .onRelease(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('!'); myPort.write('1');}});
  cp5.addButton("^2").setPosition(300,400).setSize(80,80);
  cp5.addButton("^3").setPosition(400,400).setSize(80,80);
  cp5.addButton("^4").setPosition(500,400).setSize(80,80);
  cp5.addButton("^5").setPosition(600,400).setSize(80,80);
  cp5.addButton("^ALL").setPosition(900,400).setSize(80,80);
  
  cp5.addButton("?1").setPosition(200,550).setSize(80,80);
  cp5.addButton("?2").setPosition(300,550).setSize(80,80);
  cp5.addButton("?3").setPosition(400,550).setSize(80,80);
  cp5.addButton("?4").setPosition(500,550).setSize(80,80);
  cp5.addButton("?5").setPosition(600,550).setSize(80,80);
  
  cp5.addButton("++1").setPosition(200,800).setSize(80,80);
  cp5.addButton("++2").setPosition(300,800).setSize(80,80);
  cp5.addButton("++3").setPosition(400,800).setSize(80,80);
  cp5.addButton("++4").setPosition(500,800).setSize(80,80);
  cp5.addButton("++5").setPosition(600,800).setSize(80,80);
  cp5.addButton("++ALL").setPosition(900,800).setSize(80,80);
  
  cp5.addButton("--1").setPosition(200,1000).setSize(80,80);
  cp5.addButton("--2").setPosition(300,1000).setSize(80,80);  
  cp5.addButton("--3").setPosition(400,1000).setSize(80,80);
  cp5.addButton("--4").setPosition(500,1000).setSize(80,80);
  cp5.addButton("--5").setPosition(600,1000).setSize(80,80);
  cp5.addButton("--ALL").setPosition(900,1000).setSize(80,80);
  
  cp5.addButton("POWER OFF").setPosition(0,0).setSize(150,40)
     .onPress(new CallbackListener(){public void controlEvent(CallbackEvent theEvent) {myPort.write('o'); myPort.write('0');}});
  cp5.addButton("Blue LED19").setPosition(160,0).setSize(150,40);
  cp5.addButton("Red LED17").setPosition(320,0).setSize(150,40);
  cp5.addButton("Pressure").setPosition(480,0).setSize(150,40);
  cp5.addButton("Value").setPosition(850,0).setSize(150,40);
  
  //############################################################################################   
}
void draw() {
  background(myColor);
  myColor = lerpColor(c1,c2,n);  
  
  textSize(20);
  text("Inflate:", 50,150); //inflation label
  text("maxP:", 1030,150); //inflation label
  text("Vacuum:", 50,300); 
  text("Release:", 50,450);
  text("Sense:", 50,600);
  text("Inflate2x:", 50,850);
  text("Vacuum2x:", 50,1050); 
  
 
  
  
  //LEDs
  //else if (cp5.isMouseOver(cp5.getController("Blue LED19"))){    myPort.write('b'); myPort.write('0');  }
  //else if (cp5.isMouseOver(cp5.getController("Red LED17"))){    myPort.write('r'); myPort.write('0');  }
  //else if (cp5.isMouseOver(cp5.getController("Pressure"))){
  //    myPort.write('?'); myPort.write('.'); 
  //    showPressure();
  //}


  //Otherwise
  //else  stopActionAll();
}

void stopActionAll(){
    myPort.write('!');
    myPort.write('0');
    delay(10); //helps reduce the number of serial messages being sent.
}
//The pressure readng happens only once per state. Each time a state character is sent a new pressure reading is made. Thus, all the control as far as delay
//must be set in the processing code and not in the embedded code. There must be no delays inside the state machine on the SOC ever.
void showPressure(){
  
}
