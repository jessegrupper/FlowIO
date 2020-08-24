/* The ascii character hex equivalencies are:
  ! = 0x21
  + = 0x2b
  - = 0x2d
  ^ = 0x5e
  ? = 0x3f
  p = 0x70
  n = 0x6e
*/
'use strict'
const controlServiceUUID    = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa04';
const chrCommandUUID        = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa04';
const chrHardwareStatusUUID = '0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa04';

let controlService;
let chrCommand;
let chrHardwareStatus;
let commandArray = new Uint8Array(3);
  //Initialize the array to hold the stop all command
  commandArray[0]=0x21; //'!' stop
  commandArray[1]=0xff; //all ports
  commandArray[2] = 0xff;

async function initControlService(){
  try{
    controlService = await bleServer.getPrimaryService(controlServiceUUID);
    chrCommand = await controlService.getCharacteristic(chrCommandUUID);
    chrHardwareStatus = await controlService.getCharacteristic(chrHardwareStatusUUID);
    //subscribe to receive characteristic notification events:
    await chrHardwareStatus.startNotifications();
    chrHardwareStatus.addEventListener('characteristicvaluechanged', event => { //an event is returned
      let byte0 = event.target.value.getUint8(0);
      document.querySelector('#ports').innerHTML = (byte0 & 0x1f).toBinaryString(5);
      document.querySelector('#inlet').innerHTML = (byte0>>5 & 0x01);
      document.querySelector('#outlet').innerHTML = (byte0>>6 & 0x01);
      document.querySelector('#pump1').innerHTML = (byte0>>7 & 0x01);
      let byte1 = event.target.value.getUint8(1);
      document.querySelector('#pump2').innerHTML = (byte1 & 0x01);
      document.querySelector('#ledr').innerHTML = (byte1>>1 & 0x01);
      document.querySelector('#ledb').innerHTML = (byte1>>2 & 0x01);
      document.querySelector('#sensor').innerHTML = (byte1>>3 & 0x01);
    });
    log("Control Service Initialized");

    //To get initial values for our table, we must read the hardware status characteristic.
    //which will triccer a notification to be sent, and that will populate our table.
    await getHardwareStatus();
  }
  catch(error){
    log("Ctrl Error: " + error);
    throw "ERROR: initControlService() failed.";
  }
}

//#############---CALLBACKS---###################
async function getHardwareStatus(){
  if (bleDevice && bleDevice.gatt.connected) {
    let val = await chrHardwareStatus.readValue(); //this returns a DataView
    log(val.getUint8(1).toBinaryString(8) + val.getUint8(0).toBinaryString(8));
  }
  else log("Device not connected");
}
async function startInflation(){
  if (bleDevice && bleDevice.gatt.connected) {
    commandArray[0] = 0x2b; //'+'
    commandArray[1] = getSelectedPorts();
    commandArray[2] = document.getElementById("pwm_i").value;
    await chrCommand.writeValue(commandArray);
  }
  else log("Device not connected");
}
async function startVacuum(){
  if (bleDevice && bleDevice.gatt.connected) {
    commandArray[0] = 0x2d; //'-'
    commandArray[1] = getSelectedPorts();
    commandArray[2] = document.getElementById("pwm_o").value;
    await chrCommand.writeValue(commandArray);
  }
  else log("Device not connected");
}
async function startRelease(){
  if (bleDevice && bleDevice.gatt.connected) {
    commandArray[0] = 0x5e; //'^'
    commandArray[1] = getSelectedPorts();
    commandArray[2] = 0xff; //irrelevant
    await chrCommand.writeValue(commandArray);
  }
  else log("Device not connected");
}
async function stopAllActions(){
  if (bleDevice && bleDevice.gatt.connected) {
    commandArray[0] = 0x21; //'!'
    commandArray[1] = 0xff;
    commandArray[2] = 0xff; //irrelevant
    await chrCommand.writeValue(commandArray);
  }
  else log("Device not connected");
}
async function stopAction(){
  if (bleDevice && bleDevice.gatt.connected) {
    commandArray[0] = 0x21; //'!'
    commandArray[1] = getSelectedPorts();
    commandArray[2] = 0xff; //irrelevant
    await chrCommand.writeValue(commandArray);
  }
  else log("Device not connected");
}
function getSelectedPorts(){
  let portsByte = 0x00;
  if(document.querySelector('#port1_chk').checked) portsByte ^= 0x01; //0 0001
  if(document.querySelector('#port2_chk').checked) portsByte ^= 0x02; //0 0010
  if(document.querySelector('#port3_chk').checked) portsByte ^= 0x04; //0 0100
  if(document.querySelector('#port4_chk').checked) portsByte ^= 0x08; //0 1000
  if(document.querySelector('#port5_chk').checked) portsByte ^= 0x10; //1 0000
  return portsByte;
}

//#############---PWM SLIDERS---###################
  let pwmInSlider = document.getElementById("pwm_i");
  let pwmInLabel = document.getElementById("pwm_i_label");
  pwmInLabel.innerHTML = pwmInSlider.value;
  pwmInSlider.oninput = async function(){
    let pwmInValue = this.value; //this refers to pwmInSlider.value.
    pwmInLabel.innerHTML = pwmInValue;
    if(commandArray[0]==0x2b || commandArray[0]==0x70){ //if last action is inflation ('+' or 'p'):
      commandArray[2]=pwmInValue; //Change only the PWM byte; keep the other 2 unchanged.
      //ENHANCEMENT TODO: I use a try-catch block to not display the error being shown that happens when commands
      //are being written when the device is still busy. I should instead find a way to check if the
      //device is busy and then not send the command in the first place. But current approach does the same,
      //just in a less elegant way.
      try{
        await chrCommand.writeValue(commandArray);
      }catch(error){
        if(error.message!="GATT operation already in progress.") log(error);
      }
    }
  }

  let pwmOutSlider = document.getElementById("pwm_o");
  let pwmOutLabel = document.getElementById("pwm_o_label");
  pwmOutLabel.innerHTML = pwmOutSlider.value;
  pwmOutSlider.oninput = async function(){
    let pwmOutValue = this.value; //this refers to pwmInSlider.
    pwmOutLabel.innerHTML = pwmOutValue;
    if(commandArray[0]==0x2d || commandArray[0]==0x6e){ //if last action is inflation ('-' or 'n'):
      commandArray[2]=pwmOutValue; //Change only the PWM byte; keep the other 2 unchanged.
      try{
        await chrCommand.writeValue(commandArray);
      }catch(error){
        if(error.message!="GATT operation already in progress.") log(error);
      }
    }
  }
