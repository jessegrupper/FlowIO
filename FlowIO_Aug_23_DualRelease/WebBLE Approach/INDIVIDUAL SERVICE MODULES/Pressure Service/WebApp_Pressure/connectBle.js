let bleDevice;
let bleServer;
let reconnectAttempts = 0; //Counts the number of failed reconnect attempts.

async function onConnectButtonClick() {
  let deviceOptions = {filters: [{namePrefix: DEVICE_NAME_PREFIX}],  optionalServices: listOfServices};
  //the 'listOfServices' is defined in the conditions.js file.
  try{
    bleDevice = await navigator.bluetooth.requestDevice(deviceOptions);
    bleDevice.addEventListener('gattserverdisconnected', onDisconnectEvent); //create and event lisner for disconnect events.
    bleServer = await bleDevice.gatt.connect();
    log("\nConnected!");
    try{
      await initializeAllServices();
      enableControls();
    }catch(error){
      log(error);
    }
  }
  catch(error){
    log("Connect Error: " + error);
  }
}
async function reconnect(){
  //NOTE: If we reconnect immediately after having disconnected, the reconnection will
  //happen fine, but the service initialization will fail, and then the connection
  //will drop. To fix this problem, we recursively call the reconnect() function
  //in the catch block below until we reconnect successffully or up to 3 tries.
  if (bleDevice && !bleDevice.gatt.connected){
    log("\nReconnecting...");
    try{
      await bleDevice.gatt.connect(); //connect to the same bleDevice.
    }catch(error){
      log(error);
      log("Device not found");
    }
    if (bleDevice.gatt.connected){
      log("Initializing all services...");
      disableReconnectBtn(); //Disable it as soon as we press it. It will get
      //reenabled if the reconnect fails.
      try{
        await initializeAllServices();
        enableControls();
        reconnectAttempts = 0;
      }catch(error){
        log("Reconnect failed. " + error);
        reconnectAttempts++;
        if(reconnectAttempts<3){
          log("Trying again " + reconnectAttempts)
          reconnect();
        }
      }
    }
  }
}
function onDisconnectEvent(event) { //the event argument is optional
  log("Disconnected from: " + event.target.name);
  disableControls();
  enableReconnectBtn();
}
function onDisconnectButtonClick() {
  if (!bleDevice) {
    log('No device found');
  }
  else if (bleDevice.gatt.connected) {
    log('Disconnecting...');
    bleDevice.gatt.disconnect();
  }
  else {
    log('Device already disconnected');
    disableControls();
    enableReconnectBtn();
  }
}
