/*
  This example reports battery level changes. It uses an event listener that triggers whenver
  the BLE device sends a notification from the battery level characteristic. It also shows the
  time when the event has occurred. There are many other googies you can find in the 'event' object,
  which is returned when the event fires.
    The user also has the ability to read the current in the characteristic at any time.
*/
'use strict'
const DEVICE_NAME_PREFIX = 'FlowIO';

let bleDevice;
let bleServer;

async function onConnectButtonClick() {
  try{
    bleDevice = await navigator.bluetooth.requestDevice({
          filters: [{namePrefix: DEVICE_NAME_PREFIX}],
          optionalServices: ['generic_access','battery_service']
          //You can find the names of services used by the API at
          //https://googlechrome.github.io/samples/web-bluetooth/characteristic-properties-async-await.html
          //"battery_service" and "battery_level" are names that are part of the WebBle API.
    });
    bleServer = await bleDevice.gatt.connect();
    log("Connected!");
    initBatteryService();
  }
  catch(error){
    log("Connect Error: " + error);
  }
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
