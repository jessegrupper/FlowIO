let bleDevice;
let bleServer;

async function onConnectButtonClick() {
  try{
    bleDevice = await navigator.bluetooth.requestDevice({
          filters: [{namePrefix: DEVICE_NAME_PREFIX}],
          optionalServices: listOfServices
    });
    bleServer = await bleDevice.gatt.connect();
    log("Connected!");
    await initializeAllServices();
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
