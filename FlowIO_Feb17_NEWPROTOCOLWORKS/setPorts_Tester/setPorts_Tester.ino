char val;
uint8_t pins[5] = {28,29,15,7,11};
void setup() {
  Serial.begin(115200);
}

void loop() {
  for(int i=0; i<5; i++){
    pinMode(pins[i], OUTPUT);
    pinMode(pins[i],HIGH);
  }
  if(Serial.available() > 0){
    val = Serial.read();
    setPorts(val);
    //if(val=='a') setPorts(0b00011111); 
  }
}

  void setPorts(uint8_t ports){
      //we use the 5 least significant bytes in the order from left to right.
      char portsArray[5] = {ports>>4 & 0x01, ports>>3 & 0x01, ports>>2 & 0x01, ports>>1 & 0x01, ports & 0x01};
      for(int i=0; i<5; i++){
        digitalWrite(pins[i],portsArray[i]); //element 0 of the pins array is port 1.
        Serial.print(", P");
        Serial.print(pins[i]);
        Serial.print(":!");
        Serial.print(portsArray[i],HEX);
      }
      Serial.println();
  }
