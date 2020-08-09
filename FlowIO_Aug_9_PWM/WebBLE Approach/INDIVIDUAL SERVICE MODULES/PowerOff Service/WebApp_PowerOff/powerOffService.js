/* This example powers off the flowio device and sets the power off timer.

TODO: Disable the selection box when not connected.
TODO: Disable the controls upon disconnect.
TODO: Implement if(bleDevice && bleDevice.gatt.connected) in all other services.
*/
'use strict'
const powerOffServiceUUID           = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa01';
const chrPowerOffNowUUID            = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa01';
const chrPowerOffWhenInactiveForUUID= '0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa01';
const chrPowerOffRemainingTimeUUID  = '0b0b0b0b-0b0b-0b0b-0b0b-c3000000aa01';

let powerOffService;
  let chrPowerOffNow;
  let chrPowerOffWhenInactiveFor;
  let chrPowerOffRemainingTime;

async function initPowerOffService(){
  try{
    powerOffService = await bleServer.getPrimaryService(powerOffServiceUUID);
    chrPowerOffNow = await powerOffService.getCharacteristic(chrPowerOffNowUUID);
    chrPowerOffWhenInactiveFor = await powerOffService.getCharacteristic(chrPowerOffWhenInactiveForUUID);
    //Change the embedded code so that this always hold the correct value in minutes.
    chrPowerOffRemainingTime = await powerOffService.getCharacteristic(chrPowerOffRemainingTimeUUID);

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
    throw "ERROR: initPowerOffService() failed.";
  }
}

async function getAutoOffSettingValue(){
  //Read the default auto-off value from the characteristic
  let autoOffDataView = await chrPowerOffWhenInactiveFor.readValue(); //returns a DataView
  let autoOffMinutes = autoOffDataView.getUint8(0);
  //Change the selector dropdown to show the current setting
  document.querySelector('#autoOff_select').selectedIndex = autoOffMinutes;
}
async function setAutoOffTimer(){
  let val = document.getElementById("autoOff_select").value;
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
    disableControls();
  }
  else log("Device not connected");
}
function disableControls(){
  //document.querySelector('#select').
}
