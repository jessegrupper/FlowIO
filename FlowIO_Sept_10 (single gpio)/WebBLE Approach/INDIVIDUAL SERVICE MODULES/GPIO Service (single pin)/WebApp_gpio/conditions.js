//In the HTML, you MUST load all of your service JS files before loading
//this file, because the contents of the function initializeAllService()
//are defined in those service JS files.

//#############################---INIT PARAMETERS---#################################
const DEVICE_NAME_PREFIX = 'FlowIO';
let listOfServices = ['generic_access', gpioServiceUUID];
//You can find the names of service names defined in the Web Bluetooth API at
//https://googlechrome.github.io/samples/web-bluetooth/characteristic-properties-async-await.html

//This function must contain all of the services that you wish to access:
async function initializeAllServices(){
  try{
    await initGpioService(); //defined in "gpioService.js"
  }catch(error){
    console.log("Init Error: " + error);
    throw "ERROR: initializeAllServices() failed."
  }
}

function enableControls(){
  document.querySelector('#disconnect_btn').disabled = false;
  document.querySelector('#high_btn').disabled = false;
  document.querySelector('#low_btn').disabled = false;
  document.querySelector('#readDigital_btn').disabled = false;
  document.querySelector('#readDigitalPullup_btn').disabled = false;
  document.querySelector('#readAnalog_btn').disabled = false;
  document.querySelector('#readAnalogPullup_btn').disabled = false;
  document.querySelector('#writePwm_btn').disabled = false;
  document.querySelector('#readAnalog_notify_btn').disabled = false;
  document.querySelector('#readDigital_notify_btn').disabled = false;
}
function disableControls(){
  document.querySelector('#disconnect_btn').disabled = true;
  document.querySelector('#high_btn').disabled = true;
  document.querySelector('#low_btn').disabled = true;
  document.querySelector('#readDigital_btn').disabled = true;
  document.querySelector('#readDigitalPullup_btn').disabled = true;
  document.querySelector('#readAnalog_btn').disabled = true;
  document.querySelector('#readAnalogPullup_btn').disabled = true;
  document.querySelector('#writePwm_btn').disabled = true;
  document.querySelector('#readAnalog_notify_btn').disabled = true;
  document.querySelector('#readDigital_notify_btn').disabled = true;
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
