/* The ascii character hex equivalencies are:
  ! = 0x21
  + = 0x2b
  - = 0x2d
  ^ = 0x5e
  ? = 0x3f

  TODO: Create an graphical representation based on the table.
*/
'use strict'
const controlServiceUUID    = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa04';
const chrCommandUUID        = '0b0b0b0b-0b0b-0b0b-0b0b-c1000000aa04';
const chrHardwareStatusUUID = '0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa04';

let controlService;
let chrCommand;
let chrHardwareStatus;
let valueArray = new Uint8Array(2);

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
    valueArray[0] = 0x2b; //'+'
    valueArray[1] = getSelectedPorts();
    await chrCommand.writeValue(valueArray);
  }
  else log("Device not connected");
}
async function startVacuum(){
  if (bleDevice && bleDevice.gatt.connected) {
    valueArray[0] = 0x2d; //'-'
    valueArray[1] = getSelectedPorts();
    await chrCommand.writeValue(valueArray);
  }
  else log("Device not connected");
}
async function startRelease(){
  if (bleDevice && bleDevice.gatt.connected) {
    valueArray[0] = 0x5e; //'^'
    valueArray[1] = getSelectedPorts();
    await chrCommand.writeValue(valueArray);
  }
  else log("Device not connected");
}
async function stopAllActions(){
  if (bleDevice && bleDevice.gatt.connected) {
    valueArray[0]=0x21; //'!'
    valueArray[1]=0xff;
    await chrCommand.writeValue(valueArray);
  }
  else log("Device not connected");
}
async function stopAction(){
  if (bleDevice && bleDevice.gatt.connected) {
    valueArray[0]=0x21; //'!'
    valueArray[1]=getSelectedPorts();
    await chrCommand.writeValue(valueArray);
  }
  else log("Device not connected");
}
function getSelectedPorts(){
  let portsByte = 0x00;
  if(document.querySelector('#port1').checked) portsByte ^= 0x01; //0 0001
  if(document.querySelector('#port2').checked) portsByte ^= 0x02; //0 0010
  if(document.querySelector('#port3').checked) portsByte ^= 0x04; //0 0100
  if(document.querySelector('#port4').checked) portsByte ^= 0x08; //0 1000
  if(document.querySelector('#port5').checked) portsByte ^= 0x10; //1 0000
  return portsByte;
}
