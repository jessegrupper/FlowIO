/*
  This example reports battery level changes. It uses an event listener that triggers whenver
  the BLE device sends a notification from the battery level characteristic. It also shows the
  time when the event has occurred. There are many other googies you can find in the 'event' object,
  which is returned when the event fires.
    The user also has the ability to read the current in the characteristic at any time.
*/
'use strict'
let batteryService;
let chrBatteryLevel;

async function initBatteryService(){
  try{
    batteryService = await bleServer.getPrimaryService('battery_service'); //uuid is 0x180F
    chrBatteryLevel = await batteryService.getCharacteristic('battery_level'); //uuid is 0x2A19
    //Subscribe to receive notifications from battery characteristic
    await chrBatteryLevel.startNotifications();
    chrBatteryLevel.addEventListener('characteristicvaluechanged', event => { //an event is returned
      log(event.target.value.getUint8(0)+'%');
      document.querySelector('#batLevel').innerHTML = event.target.value.getUint8(0) + '%';
      //console.log(event); //we can use this in the console to see all the goodies in the event object.
    });
    log("Battery Service Initialized");
    //To print the battery level, we simply make a read request, and that triggers
    //a notification to be sent by the device. So we don't even need to capture the
    //returned value to display it manually; just reading it is enough.
    getBatteryLevel();
  }
  catch(error){
    log("Batt Error: " + error); //NOTE: When we catch the error, execution will
    //continue and this error will not be visible by anyone who called initBatteryService().
    //Thus, to make the caller aware that initBatteryService() gave an arror, we must
    ///raise owr own error here.
    throw "ERROR: initBatteryService() failed.";
  }
}

async function getBatteryLevel(){
  if (bleDevice && bleDevice.gatt.connected){
    await chrBatteryLevel.readValue(); //returns a DataView but we don't need it
    //because it also triggers the 'characteristicvaluechanged' notification.
  }
  else log("Device not connected");
}
