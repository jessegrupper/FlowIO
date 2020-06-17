let bleDevice;
let bleServer;

async function onConnectButtonClick() {
  let deviceOptions = {filters: [{namePrefix: DEVICE_NAME_PREFIX}],  optionalServices: listOfServices};
  //the 'listOfServices' is defined in the bleParameters.js file.
  try{
    bleDevice = await navigator.bluetooth.requestDevice(deviceOptions);
    bleServer = await bleDevice.gatt.connect();
    log("Connected!");
    await initializeAllServices();
    enableControls();
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
    disableControls();
  }
  else {
    log('Device already disconnected');
    disableControls();
  }
}

function log(text) {
    console.log(text);
    document.querySelector('#log').textContent += text + '\n';
}
