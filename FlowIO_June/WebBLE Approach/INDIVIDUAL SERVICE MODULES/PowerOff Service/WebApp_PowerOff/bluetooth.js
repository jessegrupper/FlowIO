/* This example powers off the flowio device and sets the power off timer.

TODO: Disable the selection box when not connected.
TODO: Disable the controls upon disconnect.
TODO: Implement if(bleDevice && bleDevice.gatt.connected) in all other files.
*/
'use strict'
const DEVICE_NAME_PREFIX = 'FlowIO';
const powerOffServiceUUIDString           = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa01';
const chrPowerOffNowUUIDString            = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa01';
const chrPowerOffWhenInactiveForUUIDString= '0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa01';
const chrPowerOffRemainingTimeUUIDString  = '0b0b0b0b-0b0b-0b0b-0b0b-c3000000aa01';

let bleDevice;
let bleServer;

let powerOffService;
  let chrPowerOffNow;
  let chrPowerOffWhenInactiveFor;
  let chrPowerOffRemainingTime;

async function onConnectButtonClick() {
  try{
    bleDevice = await navigator.bluetooth.requestDevice({
          filters: [{namePrefix: DEVICE_NAME_PREFIX}],
          optionalServices: [powerOffServiceUUIDString]
        });
    bleServer = await bleDevice.gatt.connect();
    log("Connected");
    await initPowerOffService();
  }
  catch(error){
    log("Connect Error: " + error);
  }
}

async function initPowerOffService(){
  try{
    powerOffService = await bleServer.getPrimaryService(powerOffServiceUUIDString);
    chrPowerOffNow = await powerOffService.getCharacteristic(chrPowerOffNowUUIDString);
    chrPowerOffWhenInactiveFor = await powerOffService.getCharacteristic(chrPowerOffWhenInactiveForUUIDString);
    //Change the embedded code so that this always hold the correct value in minutes.
    chrPowerOffRemainingTime = await powerOffService.getCharacteristic(chrPowerOffRemainingTimeUUIDString);

    //Subscribe to receive the notifications for remaining time till off:
    await chrPowerOffRemainingTime.startNotifications();
    chrPowerOffRemainingTime.addEventListener('characteristicvaluechanged', event => {
      log('Remaining time: ' + event.target.value.getUint8(0));
    });

    //Subscribe to receive the notifications for changes in the timer value:
    await chrPowerOffWhenInactiveFor.startNotifications();
    chrPowerOffWhenInactiveFor.addEventListener('characteristicvaluechanged', event => {
      log('Timer set to: ' + event.target.value.getUint8(0));
    });

    log("PowerOff Service Initialized");
    getAutoOffSettingValue();
  }
  catch(error){
    log("Ouch! " + error);
  }
}

async function getAutoOffSettingValue(){
  //Read the default auto-off value from the characteristic
  let autoOffDataView = await chrPowerOffWhenInactiveFor.readValue(); //returns a DataView
  let autoOffMinutes = autoOffDataView.getUint8(0);
  //Change the selector dropdown to show the current setting
  document.querySelector('#select').selectedIndex = autoOffMinutes;
}
async function setAutoOffTimer(){
  let val = document.getElementById("select").value;
  let valArray = new Uint8Array([val]);
  await chrPowerOffWhenInactiveFor.writeValue(valArray);
}

async function powerOff(){
  if (bleDevice && bleDevice.gatt.connected) {
    //We need both of these ^^^ checks because bleDevice continues
    //to exist after we disconnect. And if we check only for the
    //second one, then we will get an error if bleDevice has
    //not been created yet. Thus we need both.
    log('Power off');
    let poweroff = new Uint8Array([1]);
    await chrPowerOffNow.writeValue(poweroff);
  }
  else log("Device not connected");
}
function disableControls(){
  //document.querySelector('#select').
}
function onDisconnectButtonClick() {
  if (!bleDevice) {
    log('No device found');
  }
  else if (bleDevice.gatt.connected) {
    log('Disconnecting');
    bleDevice.gatt.disconnect();
    disableControls();
  }
  else {
    log('Device already disconnected');
  }
}
function log(text) {
    console.log(text);
    document.querySelector('#log').textContent += text + '\n';
}
