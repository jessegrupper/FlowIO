/*This example implements the following APIs for reading and writing to pins.
readAnalog(3,INPUT)
readAnalog(3,INPUT_PULLUP)
readAnalog(3,INPUT_PULLUP,notifyOnChange)
readDigital(3,INPUT)
readDigital(3,INPUT_PULLUP)
writeDigital(3,LOW);
writeDigital(3,HIGH);
writePWM(3,123)

TODO: Add a slider to be able to set the value of the PWM.
TODO: The difference b/n input and input_pullup is not reflected in the console log. Maybe the mode is not not changing as I intend.
  When I check it independently, the value for INPUT_PULLUP is around 900 with nothing connected to the pin.
TODO: Add a chrGPIOstate characteristic with its own status bytes for the GPIO.
*/
'use strict'
const gpioServiceUUID      = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa06';
const chrPin02UUID   = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa06';

let gpioService;
let chrPin02;
let value; //this holds the array to be written to the characteristic.

async function initGpioService() {
  try{
    gpioService = await bleServer.getPrimaryService(gpioServiceUUID);
    chrPin02 = await gpioService.getCharacteristic(chrPin02UUID);

    //Subscribe to receive notifications about the pin value.
    await chrPin02.startNotifications();
    chrPin02.addEventListener('characteristicvaluechanged', event => { //an event is returned
      //The returned value is 16-bit. We can read is a one uint16 with flipped endianness,
      //or as two uint8 bytes. We choose the first option.
      let pin02Val = event.target.value.getUint16(0,true); //The 0 means we are reading the lowest 16-bits. true means we flip the endicanness.
      log("pin02Val = " + pin02Val)
    })
  }
  catch(error){
    log("GPIO Error: " + error);
    throw "ERROR: initGpioService() failed.";
  }
}

async function writeDigital(pin,pinState){
  if (bleDevice && bleDevice.gatt.connected) {
    if(pinState!=0x00) pinState=0x01; //anything other 0, set it to HIGH.
    value = new Uint8Array([pinState]);
    if(pin==2) await chrPin02.writeValue(value);
  }
  else log("Device not connected");
}
async function readDigital(pin,inputType){
  if (bleDevice && bleDevice.gatt.connected) {
    if(inputType=="INPUT_PULLUP")      value = new Uint8Array([0x03]);
    else value = new Uint8Array([0x02]);
    if(pin==2) await chrPin02.writeValue(value);
    //It is not unnecessary to read the characteristic, b/c it sends a notification automatically.
    //that we are capturing already.
    //let pinVal = awaitchrPin02.readValue();
    //return pinVal;
  }
  else log("Device not connected");
}
async function readDigitalNotify(pin,inputType){
  if (bleDevice && bleDevice.gatt.connected) {
    if(inputType=="INPUT_PULLUP") value = new Uint8Array([0x05]);
    else value = new Uint8Array([0x04]);
    if(pin==2) await chrPin02.writeValue(value)
    //It is not unnecessary to read the characteristic, b/c it sends a notification automatically.
    //that we are capturing already.
    //let pinVal = awaitchrPin02.readValue();
    //return pinVal;
  }
  else log("Device not connected");
}
async function readAnalog(pin,inputType){
  if (bleDevice && bleDevice.gatt.connected) {
    if(inputType=="INPUT_PULLUP") value = new Uint8Array([0x09]);
    else value = new Uint8Array([0x08]);
    if(pin==2) await chrPin02.writeValue(value)
    //It is not unnecessary to read the characteristic, b/c it sends a notification automatically.
    //that we are capturing already.
    //let pinVal = awaitchrPin02.readValue();
    //return pinVal;
  }
  else log("Device not connected");
}
async function readAnalogNotify(pin,inputType){
  if (bleDevice && bleDevice.gatt.connected) {
    if(inputType=="INPUT_PULLUP") value = new Uint8Array([0x07]);
    else value = new Uint8Array([0x06]);
    if(pin==2) await chrPin02.writeValue(value)
    //It is not unnecessary to read the characteristic, b/c it sends a notification automatically.
    //that we are capturing already.
    //let pinVal = awaitchrPin02.readValue();
    //return pinVal;
  }
  else log("Device not connected");
}
async function writePWM(pin,pwmValue){
  if (bleDevice && bleDevice.gatt.connected) {
    if(pwmValue>255) pwmValue=255;
    else if(pwmValue<0) pwmValue=0;
    value = new Uint8Array([0x0A,pwmValue]);
    if(pin==2) await chrPin02.writeValue(value);
  }
  else log("Device not connected");
}
