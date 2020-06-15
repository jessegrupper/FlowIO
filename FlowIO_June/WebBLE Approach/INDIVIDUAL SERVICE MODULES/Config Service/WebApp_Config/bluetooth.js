'use strict'
const DEVICE_NAME_PREFIX = 'FlowIO';
const configServiceUUID = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa03';
const chrConfigUUID     = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa03';

let bleDevice;
let bleServer;
let configService;
let chrConfig;

window.onload = function(){
  document.querySelector('#select').addEventListener('change', setConfiguration);
};

async function onConnectButtonClick() {
  try{
    bleDevice = await navigator.bluetooth.requestDevice({
          filters: [{namePrefix: DEVICE_NAME_PREFIX}],
          optionalServices: [configServiceUUID]
        });
    bleServer = await bleDevice.gatt.connect();
    log("Connected");
    initConfigService();
  }
  catch(error){
    log("Connect Error: " + error);
  }
}

async function initConfigService(){
  try {
    configService = await bleServer.getPrimaryService(configServiceUUID);
    chrConfig = await configService.getCharacteristic(chrConfigUUID);
    log("Config Service Initialized");
    getConfiguration();
  }
  catch (error){
    log("Init Error: " + error);
  }
}

async function getConfiguration(){
  try{
    let config = await chrConfig.readValue(); //this returns a DataView
    let configNumber = config.getUint8(0);
    //The configurations have been encoded into numbers b/n 0 and 4.
    if(configNumber==0){
      log('GENERAL');
      document.querySelector("#currentConfig").innerHTML = "GENERAL";
    }else if(configNumber==1){
      log('INFLATION_SERIES');
      document.querySelector("#currentConfig").innerHTML = "INFLATION_SERIES";
    }else if(configNumber==2){
      log('INFLATION_PARALLEL');
      document.querySelector("#currentConfig").innerHTML = "INFLATION_PARALLEL";
    }else if(configNumber==3){
      log('VACUUM_SERIES');
      document.querySelector("#currentConfig").innerHTML = "VACUUM_SERIES";
    }else if(configNumber==4){
      log('VACUUM_PARALLEL');
      document.querySelector("#currentConfig").innerHTML = "VACUUM_PARALLEL";
    }
  }
  catch(error){
    log("Get Error: " + error);
  }
}
async function setConfiguration(){
  //1. If the user selects the empty element, I want to preserve the configuration the same.
  //2. I want the console window to display the name of the configuration not the value of it.
  let val = document.getElementById("select").value;

  if(val!=''){
    log('Changed to: ')
    let valArray = new Uint8Array([val]);
    await chrConfig.writeValue(valArray);
  }
  if(val==0){
    log('GENERAL');
    document.querySelector("#currentConfig").innerHTML = "GENERAL";
  }else if(val==1){
    log('INFLATION_SERIES');
    document.querySelector("#currentConfig").innerHTML = "INFLATION_SERIES";
  }else if(val==2){
    log('INFLATION_PARALLEL');
    document.querySelector("#currentConfig").innerHTML = "INFLATION_PARALLEL";
  }else if(val==3){
    log('VACUUM_SERIES');
    document.querySelector("#currentConfig").innerHTML = "VACUUM_SERIES";
  }else if(val==4){
    log('VACUUM_PARALLEL');
    document.querySelector("#currentConfig").innerHTML = "VACUUM_PARALLEL";
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
