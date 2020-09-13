
void setup() {
  Serial.begin(115200);
  analogReference(AR_INTERNAL_3_0);   // Set the analog reference to 3.0V (default = 3.6V) b/c battery voltage may drop to <3.6V and then default will become inaccurate.
  analogReadResolution(12);   // Set the resolution to 12-bit (0..4095). Can be 8, 10, 12 or 14
}

void loop() {
  Serial.println(getStrain32bit(),BIN);
  delay(2000);
}

uint8_t getVoltage8bit(uint8_t pin) {
  int voltage0to2095 = analogRead(pin);   // Get the raw 12-bit, 0..3000mV ADC value  
  float voltage0to255float = voltage0to2095*255.0/4095.0;
  //A conversion from float to int trunkates the result. 
  //If we add 0.5 to the float value before the trunkating conversion, then we actually
  //achieve rouding. For example, say val=3.6. If we convert directly, it would become 3.
  //But if we add 0.5 first and then convert it becomes 4.1 and convert to 4.
  uint8_t voltage8bit = (uint8_t) (voltage0to255float+0.5);
  
  Serial.print(voltage0to2095);
  Serial.print("\t <--->\t");
  Serial.println(voltage8bit);
  
  return voltage8bit; 
}

uint32_t getStrain32bit(){
  uint32_t strain32bit;
  strain32bit = getVoltage8bit(3);   //byte 3
  strain32bit <<= 8;
  strain32bit |= getVoltage8bit(4);  //byte 2
  strain32bit <<= 8;
  strain32bit |= getVoltage8bit(5);  //byte 1
  strain32bit <<= 8;
  strain32bit |= getVoltage8bit(28);  //byte 0
  return strain32bit;
}

//TODO: test whether the output value is actually the correct voltage.  I need 
//and extern la power source. If it is not, then I need to adjust hte analogReference.
