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
