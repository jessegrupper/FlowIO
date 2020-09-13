/*
  This example shows how to control the state of each of the two LEDs on the nrf52832 Feather
  board. There is a characteristic that holds a 2-byte value, each byte corresponds to
  the state of an LED. The characteristic has a read and write permissions.
    In this JavaScript code, we are connecting to the device and reading the current value of
  the characteristic. Then we are toggling one of the two bytes depending on which button the
  user presses on the screen, which in turn causes the corresponding LED to toggle as well.
*/
'use strict'
const indicatorServiceUUID = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa02';
const chrLedStatesUUID     = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa02';
const chrErrorUUID         = '0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa02';

let indicatorService;
let chrLedStates;
let chrError;
let ledStatesArray;
let stateRed = true;
let stateBlue = true;

async function initIndicatorService(){
  try{
    indicatorService = await bleServer.getPrimaryService(indicatorServiceUUID);
    chrLedStates = await indicatorService.getCharacteristic(chrLedStatesUUID);
    chrError = await indicatorService.getCharacteristic(chrErrorUUID);

    //Subscribe to receive notifications from the error characteristic
    await chrError.startNotifications();
    chrError.addEventListener('characteristicvaluechanged', event => {
      log("Error Code: " + event.target.value.getUint8(0));
    });

    //Subscribe to receive notifications from chrLedStates.
    await chrLedStates.startNotifications(); //This causes red LED to turn off
    //for unknown reasons having to do with the nrf52 bootloader or OS.
    chrLedStates.addEventListener('characteristicvaluechanged', event => {
      log("Notification: B=" + event.target.value.getUint8(1) + " R=" + event.target.value.getUint8(0));
    })
    log("indicator Service Initialized");
    //Make read requests to trigger our notification funcion and to get initial values.
    getLedStates();
    readError();
  }
  catch(error){
    log("Init Error: " + error);
    throw "ERROR: initIndicatorService() failed.";
  }
}

async function getLedStates(){
  if (bleDevice && bleDevice.gatt.connected) {
    let valueDataView = await chrLedStates.readValue(); //returns a DataView and
    //triggers a notification. Hence, we don't need to log the value of 'val'
    //here because we will get it in the event listener function above.
    //(If you didn't know that object is of type "DataView" you could just do
    //console.log(valueDataView) and then you will see all info about it in the console.

    //Set our LED state variables to match those in the characteristic:
    //We now convert the DataView to TypedArray so we can use array notation to access the data.
    //https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/DataView/buffer
    ledStatesArray = new Uint8Array(valueDataView.buffer);
    stateBlue = ledStatesArray[1];
    stateRed = ledStatesArray[0];
  }
  else log("Device not connected");
}
async function readError(){
  if (bleDevice && bleDevice.gatt.connected) {
    await chrError.readValue(); //this will trigger our notification listener.
  }
  else log("Device not connected");
}
async function clearError(){
  if (bleDevice && bleDevice.gatt.connected) {
    let zeroArray = new Uint8Array([0]);
    await chrError.writeValue(zeroArray);
  }
  else log("Device not connected");
}
async function toggleRed(){
  if (bleDevice && bleDevice.gatt.connected) {
    ledStatesArray[0] = (stateRed) ? 0x00 : 0x01;
    stateRed = !stateRed;
    await chrLedStates.writeValue(ledStatesArray);
  }
  else log("Device not connected");
}
async function toggleBlue(){
  if (bleDevice && bleDevice.gatt.connected) {
    ledStatesArray[1] = (stateBlue) ? 0x00 : 0x01;
    stateBlue = !stateBlue;
    await chrLedStates.writeValue(ledStatesArray);
  }
  else log("Device not connected");
}
