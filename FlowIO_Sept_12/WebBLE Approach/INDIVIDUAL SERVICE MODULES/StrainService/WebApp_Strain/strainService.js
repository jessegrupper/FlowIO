/*
  This example reports strain changes. It uses an event listener that triggers whenver
  the BLE device sends a notification from the strain characteristic. It also shows the
  time when the event has occurred. There are many other googies you can find in the 'event' object,
  which is returned when the event fires.
    The user also has the ability to read the current in the characteristic at any time.
*/
'use strict'
const strainServiceUUID = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa10';
const chrStrainUUID     = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa10';

let strainService;
let chrStrain;

async function initStrainService(){
  try{
    strainService = await bleServer.getPrimaryService(strainServiceUUID);
    chrStrain = await strainService.getCharacteristic(chrStrainUUID);
    //Subscribe to receive notifications characteristic
    await chrStrain.startNotifications();
    chrStrain.addEventListener('characteristicvaluechanged', event => { //an event is returned
      //TODO: Parse the 32 bit value into 4 8bit values.
      log('pin29: ' + event.target.value.getUint8(0) + '\tpin5: ' + event.target.value.getUint8(1) + '\tpin4: ' + event.target.value.getUint8(2) + '\tpin3: ' + event.target.value.getUint8(3)); //byte 0 (lowest 8bits)
      //document.querySelector('#strainVal').innerHTML = event.target.value.getUint8(0) + ' zeroth';
      //console.log(event); //we can use this in the console to see all the goodies in the event object.
    });
    log("Strain Service Initialized");
    //To print the strain, we simply make a read request, and that triggers
    //a notification to be sent by the device. So we don't even need to capture the
    //returned value to display it manually; just reading it is enough.
    getStrain();
  }
  catch(error){
    log("Strain Error: " + error); //NOTE: When we catch the error, execution will
    //continue and this error will not be visible by anyone who called initStrainService().
    //Thus, to make the caller aware that initStrainService() gave an arror, we must
    ///raise owr own error here.
    throw "ERROR: initStrainService() failed.";
    //Anything we put here after "throw" line will not get executed.
  }
}

async function getStrain(){
  if (bleDevice && bleDevice.gatt.connected){
    await chrStrain.readValue(); //returns a DataView but we don't need it
    //because it also triggers the 'characteristicvaluechanged' notification.
  }
  else log("Device not connected");
}
