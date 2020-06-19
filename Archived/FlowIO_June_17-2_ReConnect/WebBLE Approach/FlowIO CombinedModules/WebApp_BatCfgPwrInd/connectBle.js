let bleDevice;
let bleServer;

async function onConnectButtonClick() {
  let deviceOptions = {filters: [{namePrefix: DEVICE_NAME_PREFIX}],  optionalServices: listOfServices};
  //the 'listOfServices' is defined in the bleParameters.js file.
  try{
    bleDevice = await navigator.bluetooth.requestDevice(deviceOptions);
    bleDevice.addEventListener('gattserverdisconnected', onDisconnectEvent); //create and event lisner for disconnect events.
    bleServer = await bleDevice.gatt.connect();
    log("Connected!");
    await initializeAllServices();
    enableControls();
  }
  catch(error){
    log("Connect Error: " + error);
  }
}
async function reconnect(){
  //Will not work if you try reconnecting as soon as you disconnect, because
  //the device will not yet have become discoverable. Thus, I should first check
  //if the bleDevice is advertising and "while" it is not advertising, I should just
  //wait, and as soon as it starts advertising, I exist the waiting loop, and I try
  //reconnecting with it right away.
  await bleDevice.gatt.connect(); //connect to the same bleDevice.
  log("Reconnected");
  enableControls();
  batteryService = null;
  chrBatteryLevel = null;
  if (bleDevice.gatt.connected){
    await initializeAllServices();
    log("executed");
  }

}
function onDisconnectEvent(event) { //the event argument is optional
  disableControls();
  log("Disconnected from: " + event.target.name);
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
