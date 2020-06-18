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

let bleDevice;
let bleServer;
let controlService;
let chrCommand;
let chrHardwareStatus;
let valueArray = new Uint8Array(2);

let arrayA = new Uint8Array(2);
let arrayB = new Uint8Array(2);
let arrayC = new Uint8Array(2);
let arrayD = new Uint8Array(2);
let arrayE = new Uint8Array(2);



window.onload = function(){
  document.querySelector('#connect').addEventListener('click', connect);
  document.querySelector('#disconnect').addEventListener('click', onDisconnectButtonClick);
  document.querySelector('#inflate').addEventListener('click', startInflation);
  document.querySelector('#vacuum').addEventListener('click', startVacuum);
  document.querySelector('#release').addEventListener('click', startRelease);
  document.querySelector('#stopAll').addEventListener('click', stopAllActions);
  document.querySelector('#stop').addEventListener('click', stopAction);
  document.querySelector('#read').addEventListener('click', getHardwareStatus);

  document.getElementById("execute").addEventListener('click', executeSequence);
  document.getElementById("addrow").addEventListener('click', addRow);
  document.getElementById("deleterow").addEventListener('click', deleteRow);
  addRow(); //we are creating the firt row of the table dynamically on load.
};



//#############################################################################################################################
function executeSequence(){
  let scheduleTable = document.getElementById("schedule");
  let rows=scheduleTable.rows.length; //because the title row is not a line item.

  
  for(let i=1; i<rows-1; i++){ //we don't want the last row. We do it separately after the loop.
    let time = document.getElementById(`time_${i}`).value;
    let action = document.getElementById(`action_${i}`).value;
    executeAction(time,action,i);
  }

  //now we execute the last row separately because its id's are having the ending 0.
  let i=0; //this is the last row of the table.
  let time = document.getElementById(`time_${i}`).value;
  let action = document.getElementById(`action_${i}`).value;
  executeAction(time,action,i);

}

function executeAction(time,action,lineItem){ //the time specifies AT WHICH TIME the action will begin, NOT the duration of the action.
  //In this functions is where I will be calling the setTimeout() function 
  let actionArray = new Uint8Array(2);
  if(action=="+")        actionArray[0] = 0x2b; //'+'
  else if(action == "-") actionArray[0] = 0x2d; //'-'
  else if(action == "^") actionArray[0] = 0x5e; //'^'
  else                   actionArray[0] = 0x21; //'!'
  
  actionArray[1] = getPortsByte(lineItem);
  setTimeout(async function(){await chrCommand.writeValue(actionArray);}, parseInt(time));
}

function getPortsByte(i){
  let portsByte = 0x00;
    if(document.querySelector(`#port1_${i}`).checked) portsByte ^= 0x01; //0 0001
    if(document.querySelector(`#port2_${i}`).checked) portsByte ^= 0x02; //0 0010
    if(document.querySelector(`#port3_${i}`).checked) portsByte ^= 0x04; //0 0100
    if(document.querySelector(`#port4_${i}`).checked) portsByte ^= 0x08; //0 1000
    if(document.querySelector(`#port5_${i}`).checked) portsByte ^= 0x10; //1 0000
  log(portsByte);
  return portsByte;
}

//I don't like the whole structure of functions calling functions calling functions. 
//There must be a better way of doing this. But my javascript is quite weak.
//If I was doing this in c++, how would I do it with object oriented approach?

//#############################################################################################################################
async function connect() {  
  try{
    bleDevice = await navigator.bluetooth.requestDevice({
          filters: [{namePrefix: 'nrf52'}],
          optionalServices: [controlServiceUUID]
        });
    bleServer = await bleDevice.gatt.connect();
    controlService = await bleServer.getPrimaryService(controlServiceUUID);
    chrCommand = await controlService.getCharacteristic(chrCommandUUID);  
    chrHardwareStatus = await controlService.getCharacteristic(chrHardwareStatusUUID);
    log("Connected");

    //subscribe to receive characteristic notification events
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

    })

    //read the value of the chrHardwareStatus. This will trigger a notification to be sent.
    //This is critical, because it initilizes the values in our html table upon connect.
    let valueDataView = await chrHardwareStatus.readValue(); //returns a DataView.
  }

  catch(error){
    log("Ouch! " + error);
  }
}


//#############---CALLBACKS---###################
function deleteRow(){ //delete the previous to the last row.
  let scheduleTable = document.getElementById("schedule");
  let i=scheduleTable.rows.length; 
  if(i>2) scheduleTable.deleteRow(i-2); //the title row is number 0.
}
function addRow() {
  let scheduleTable = document.getElementById("schedule");

  let i=scheduleTable.rows.length-1; //the first row of the table is title row.
  let row = scheduleTable.insertRow(i); //I want to add the new row before the last one.
  //log(i);
  row.insertCell(0).innerHTML = `<input id="time_${i}" type="number" min="0" max="20000" value="0" step="100"><span> ms</span>`;
  row.insertCell(1).innerHTML = `
                    <select id="action_${i}">
                      <option value="+">Start Inflation</option>
                      <option value="-">Start Vacuum</option>
                      <option value="^">Start Release</option>
                      <option value="!">Stop Action</option>
                    </select>
                    `;
  row.insertCell(2).innerHTML = `
                <select id="device_${i}" value="1" disabled>
                  <option value="1">FlowIO 1</option>
                  <option value="2">FlowIO 2</option>
                  <option value="3">FlowIO 3</option>
                  <option value="4">FlowIO 4</option>
                  <option value="5">FlowIO 5</option>
                </select>
                `; 
row.insertCell(3).innerHTML = `
              <div class="form-check form-check-inline">
                <label class="form-check-label" for="inlineCheckbox1">1</label>
                <input class="form-check-input" type="checkbox" id="port1_${i}" value="1">
                <label class="form-check-label" for="inlineCheckbox2">2</label>
                <input class="form-check-input" type="checkbox" id="port2_${i}" value="2">
                <label class="form-check-label" for="inlineCheckbox2">3</label>
                <input class="form-check-input" type="checkbox" id="port3_${i}" value="3">
                <label class="form-check-label" for="inlineCheckbox2">4</label>
                <input class="form-check-input" type="checkbox" id="port4_${i}" value="4">
                <label class="form-check-label" for="inlineCheckbox2">5</label>
                <input class="form-check-input" type="checkbox" id="port5_${i}" value="5">
              </div> 
              `;            
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
async function getHardwareStatus(){
  let val = await chrHardwareStatus.readValue(); //this returns a DataView
  log(val.getUint8(1).toBinaryString(8) + val.getUint8(0).toBinaryString(8));
}
async function startInflation(){
  valueArray[0] = 0x2b; //'+'
  valueArray[1] = getSelectedPorts();
  await chrCommand.writeValue(valueArray);
}
async function startVacuum(){
  valueArray[0] = 0x2d; //'-'
  valueArray[1] = getSelectedPorts();
  await chrCommand.writeValue(valueArray);
}
async function startRelease(){
  valueArray[0] = 0x5e; //'^'
  valueArray[1] = getSelectedPorts();
  await chrCommand.writeValue(valueArray);
}
async function stopAllActions(){
  valueArray[0]=0x21; //'!'
  valueArray[1]=0xff;
  await chrCommand.writeValue(valueArray);
}
async function stopAction(){
  valueArray[0]=0x21; //'!'
  valueArray[1]=getSelectedPorts();
  await chrCommand.writeValue(valueArray);
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
function log(text) {
    console.log(text);
    document.querySelector('#log').textContent += text + '\n';
}