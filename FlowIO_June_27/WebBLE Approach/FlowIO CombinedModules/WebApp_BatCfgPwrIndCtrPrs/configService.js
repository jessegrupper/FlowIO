/*Implements access to the BLE configuration service of the FlowIO.
The configuration is retrieved upon connecting to the FlowIO and selectedIndex
is changed to reflect the current configuration. The FLowIO encodes the
5 configurations simply as numbers between 0 and 4, and it is in this JS
code that we give names to those numbers. We are simply receing a single-byte
number from the FlowIO when we read the configuration.
*/

'use strict'
const configServiceUUID = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa03';
const chrConfigUUID     = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa03';

let configService;
let chrConfig;

//TODO: Make the selector disabled if the device is not connected.

async function initConfigService(){
  try {
    configService = await bleServer.getPrimaryService(configServiceUUID);
    chrConfig = await configService.getCharacteristic(chrConfigUUID);
    log("Config Service Initialized");
    getConfiguration();
  }
  catch (error){
    log("Init Error: " + error);
    throw "ERROR: initConfigService() failed.";
  }
}

async function getConfiguration(){
  try{
    //1. Get the current configuration.
    let config = await chrConfig.readValue(); //this returns a DataView
    let configNumber = config.getUint8(0);
    //2. Make the selector to display the current configuration.
    document.querySelector('#config_select').selectedIndex = configNumber;
    //3. Print the current configuration to the log and to the HTML.
    //The configurations have been encoded into numbers b/n 0 and 4.
    if(configNumber==0)       log('GENERAL');
    else if(configNumber==1)  log('INFLATION_SERIES');
    else if(configNumber==2)  log('INFLATION_PARALLEL');
    else if(configNumber==3)  log('VACUUM_SERIES');
    else if(configNumber==4)  log('VACUUM_PARALLEL');
  }
  catch(error){
    log("Get Error: " + error);
  }
}
async function setConfiguration(){
  let val = document.getElementById("config_select").value;
  let valArray = new Uint8Array([val]);
  await chrConfig.writeValue(valArray);
  log('Changed to: ');
  if(val==0)        log('GENERAL');
  else if(val==1)   log('INFLATION_SERIES');
  else if(val==2)   log('INFLATION_PARALLEL');
  else if(val==3)   log('VACUUM_SERIES');
  else if(val==4)   log('VACUUM_PARALLEL');
}
