/*
  This example shows how to control the state of each of the two LEDs on the nrf52832 Feather
  board. The microcontorller has been set up so that is has one custom service with a single
  characteristic that holds a 2-byte value. The two bytes in the characteristic correspond to
  the state of the two LEDs on the board. The characteristic has a read and write persmisions.
    In this JavaScript code, we are connecting to the device and reading the current value of
  the characteristic. Then we are toggling one of the two bytes depending on which button the
  user presses on the screen, which in turn causes the corresponding LED to toggle as well.

  This example uses the Async / Await approach.
*/
'use strict'
const DEVICE_NAME_PREFIX = 'FlowIO';
const indicatorServiceUUID = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa02';
const chrLedStatesUUID     = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa02';
const chrErrorUUID         = '0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa02';

let bleDevice;
let bleServer;
let indicatorService;
let chrLedStates;
let chrError;
let valueArray;
let stateRed = true;
let stateBlue = true;

async function onConnectButtonClick() {
  try{
    bleDevice = await navigator.bluetooth.requestDevice({
          filters: [{namePrefix: DEVICE_NAME_PREFIX}],
          optionalServices: ['generic_access', indicatorServiceUUID] //you can also write the UUID here.
          //You can find the names of services used by the API at
          //https://googlechrome.github.io/samples/web-bluetooth/characteristic-properties-async-await.html
        });
    bleServer = await bleDevice.gatt.connect();
    log("Connected");
    initIndicatorService();
  }
  catch(error){
    log("Connect Error: " + error);
  }
}

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
    valueArray = new Uint8Array(valueDataView.buffer);
    stateBlue = valueArray[1];
    stateRed = valueArray[0];
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
    valueArray[0] = (stateRed) ? 0x00 : 0x01;
    stateRed = !stateRed;
    await chrLedStates.writeValue(valueArray);
  }
  else log("Device not connected");
}
async function toggleBlue(){
  if (bleDevice && bleDevice.gatt.connected) {
    valueArray[1] = (stateBlue) ? 0x00 : 0x01;
    stateBlue = !stateBlue;
    await chrLedStates.writeValue(valueArray);
  }
  else log("Device not connected");
}

function onDisconnectButtonClick() {
  if (!bleDevice) {
    log('No device found');
  }
  else if (bleDevice.gatt.connected) {
    log('Disconnecting');
    bleDevice.gatt.disconnect();
  }
  else {
    log('Device already disconnected');
  }
}
function log(text) {
    console.log(text);
    document.querySelector('#log').textContent += text + '\n';
}
