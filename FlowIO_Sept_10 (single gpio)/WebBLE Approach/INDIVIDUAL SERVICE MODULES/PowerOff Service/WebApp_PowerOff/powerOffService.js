/* This example powers off the flowio device and sets the power off timer.
*/
'use strict'
const powerOffServiceUUID = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa01';
const chrPowerOffTimerUUID = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa01';

let powerOffService;
let chrPowerOffTimer;

async function initPowerOffService(){
  try{
    powerOffService = await bleServer.getPrimaryService(powerOffServiceUUID);
    chrPowerOffTimer = await powerOffService.getCharacteristic(chrPowerOffTimerUUID);

    //Subscribe to receive the notifications
    await chrPowerOffTimer.startNotifications();
    chrPowerOffTimer.addEventListener('characteristicvaluechanged', event => {
      let remainingTime = event.target.value.getUint8(0);
      if(remainingTime==0xFF) log("AutoOFF Disabled");
      else log('Remaining time: ' + remainingTime);
    });

    log("PowerOff Service Initialized");
    let initTimer = await getRemainingTime(); //this triggers a notification event. (Doesn't work without await!)
    document.querySelector('#autoOff_select').selectedIndex = initTimer; //sets the selector box to the initTimer value.
  }
  catch(error){
    log("Ouch! " + error);
    throw "ERROR: initPowerOffService() failed.";
  }
}

async function getRemainingTime(){
  let minutesDataView = await chrPowerOffTimer.readValue(); //returns a DataView
  let minutes = minutesDataView.getUint8(0);
  return minutes;
  //Change the selector dropdown to show the current setting. This is only needed when we first connect.
  //document.querySelector('#autoOff_select').selectedIndex = minutes;
}
async function setTimer(min){
  if(min==0){
    log('Power off');
    let poweroff = new Uint8Array([0]);
    await chrPowerOffTimer.writeValue(poweroff);
    disableControls();
  }
  else{ //ignore the argument if not 0.
    let val = document.getElementById("autoOff_select").value;
    let valArray = new Uint8Array([val]);
    await chrPowerOffTimer.writeValue(valArray);
  }
}

//If you even neet to check if connection is present this is the way:
// if (bleDevice && bleDevice.gatt.connected)
//We need both of these checks ^^^ because bleDevice continues
//to exist after we disconnect. And if we check only for the
//second one, then we will get an error if bleDevice has
//not been created yet. Thus we need both.
