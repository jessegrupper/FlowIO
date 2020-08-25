//In the HTML, you MUST load all of your service JS files before loading
//this file, because the contents of the function initializeAllService()
//are defined in those service JS files.

//#############################---INIT PARAMETERS---#################################
const DEVICE_NAME_PREFIX = 'FlowIO';
let listOfServices = ['generic_access','battery_service', powerOffServiceUUID];
//You can find the names of service names defined in the Web Bluetooth API at
//https://googlechrome.github.io/samples/web-bluetooth/characteristic-properties-async-await.html

//This function must contain all of the services that you wish to access:
async function initializeAllServices(){
  try{
    await initPowerOffService(); //defined in "controlService.js"
  }catch(error){
    console.log("Init Error: " + error);
    throw "ERROR: initializeAllServices() failed."
  }
}

function enableControls(){
  document.querySelector('#disconnect_btn').disabled = false;
  document.querySelector('#autoOff_select').disabled = false;
  document.querySelector('#powerOff_btn').disabled = false;
  document.querySelector('#timeRemaining_btn').disabled = false;
}
function disableControls(){
  document.querySelector('#disconnect_btn').disabled = true;
  document.querySelector('#autoOff_select').disabled = true;
  document.querySelector('#powerOff_btn').disabled = true;
  document.querySelector('#timeRemaining_btn').disabled = true;
}
function enableReconnectBtn(){
  document.querySelector('#reconnect_btn').disabled = false;
}
function disableReconnectBtn(){
  document.querySelector('#reconnect_btn').disabled = true;
}

function log(text) {
    console.log(text);
    document.querySelector('#log').textContent += text + '\n';
}

function clearLog() {
    document.querySelector('#log').textContent = "";
}
//##################################################################################
