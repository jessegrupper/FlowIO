/*This example allows you to read and receive a notifications from a BLE device
that reports a floating point value (4-byte).

The default value reported has many digits after the decimal point, which are
not true, thus we are trunkating the value to just 5 digits.

*/
'use strict'
const DEVICE_NAME_PREFIX = 'FlowIO';
const pressureServiceUUIDString      = '0b0b0b0b-0b0b-0b0b-0b0b-00000000aa05';
const chrPressureValueUUID           = 0X2A6D;
const chrPressureRequestUUIDString   = '0b0b0b0b-0b0b-0b0b-0b0b-c2000000aa05';
const chrMaxPressureLimitsUUIDString = '0b0b0b0b-0b0b-0b0b-0b0b-c3000000aa05';
const chrMinPressureLimitsUUIDString = '0b0b0b0b-0b0b-0b0b-0b0b-c4000000aa05';

let bleDevice;
let bleServer;
let pressureService;
let chrPressureValue;
let chrPressureRequest;
let chrMaxPressureLimits;
let chrMinPressureLimits;

let maxDisplay;
let maxSlider;
let minDisplay;
let minSlider;

window.onload = function(){
  document.querySelector('#connect').addEventListener('click', connect);
  document.querySelector('#disconnect').addEventListener('click', onDisconnectButtonClick);
  document.querySelector('#read').addEventListener('click', readCharacteristicValue);
  document.querySelector('#generate').addEventListener('click', requestNewNumber);

  document.querySelector('#getmax').addEventListener('click', getMaxLimits);
  document.querySelector('#getmin').addEventListener('click', getMinLimits);
  document.querySelector('#setminmax').addEventListener('click', setMinMax);
};

function getMaxValue() {
  maxDisplay.innerHTML = this.value;
  return this.value;
}
function getMinValue() {
  minDisplay.innerHTML = this.value;
  return this.value;
}

async function connect() {
  try{
    bleDevice = await navigator.bluetooth.requestDevice({
          filters: [{namePrefix: DEVICE_NAME_PREFIX}],
          optionalServices: [pressureServiceUUIDString]
    });
    bleServer = await bleDevice.gatt.connect();
    pressureService = await bleServer.getPrimaryService(pressureServiceUUIDString);
    chrPressureValue = await pressureService.getCharacteristic(chrPressureValueUUID);
    chrPressureRequest = await pressureService.getCharacteristic(chrPressureRequestUUIDString);
    chrMaxPressureLimits = await pressureService.getCharacteristic(chrMaxPressureLimitsUUIDString);
    chrMinPressureLimits = await pressureService.getCharacteristic(chrMinPressureLimitsUUIDString);

    //This is reading the value of the characteristic and displaying it
    let valueDataView = await chrPressureValue.readValue(); //returns a DataView.
    //We must convert to a float32Array to get the endianness correctly.
        //let floatArray = new Float32Array(valueDataView.buffer);
        //log("Connected!\n" + floatArray[0] + "%");
    //Or alternatively, we can set the second parameter in the .getFloat32() to 'true'
    log("Connected!\nP = " + valueDataView.getFloat32(0,true).toFixed(3));


    //This is responding to notifications sent by the peripheral of changes to the value.
    await chrPressureValue.startNotifications();
    chrPressureValue.addEventListener('characteristicvaluechanged', event => { //an event is returned
      let floatValue = event.target.value.getFloat32(0,true);
      floatValue = floatValue.toFixed(3); //rounding to 4 decimal places.
      log("P = " + floatValue);
      console.log(event); //we can use this in the console to see all the goodies in the event object.
    })
      //NOTE: The characteristic value is automatically passed to the callback function
  }
  catch(error){
    log("Ouch! " + error);
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
async function readCharacteristicValue(){
  let val = await chrPressureValue.readValue(); //this returns a DataView
  //It is unnecessary to log the value of val, because this triggers our event listener
  //in the 'connect' function. Hence we commend out the line below.
  //log(val.getUint8(0) + "%");
}

//##################################################################################

async function setMinMax(){
  let minBuffer = new ArrayBuffer(20);
  let minArray = new Float32Array(minBuffer); //creates a view that treats the data in the buffer as float 32-bit
  minArray[0] = $("#slider-range").slider("values",0);
  minArray[1] = 1.11;
  minArray[2] = 2.22;
  minArray[3] = 3.33;
  minArray[4] = 4.44;

  let maxBuffer = new ArrayBuffer(20);
  let maxArray = new Float32Array(maxBuffer); //creates a view that treats the data in the buffer as float 32-bit
  maxArray[0] = $("#slider-range").slider("values",1);
  maxArray[1] = 9.1;
  maxArray[2] = 9.2;
  maxArray[3] = 9.3;
  maxArray[4] = 9.4;

  await chrMinPressureLimits.writeValue(minBuffer); //This can be either minBuffer or minArray
  await chrMaxPressureLimits.writeValue(maxBuffer);
  for(let i=0; i<5; i++)
    log(minArray[i].toFixed(2) + ' - ' + maxArray[i].toFixed(2));
}


async function getMinLimits(){
  let val = await chrMinPressureLimits.readValue(); //returns a dataView
  let min5 = val.getFloat32(0,true);
  let min4 = val.getFloat32(4,true);
  let min3 = val.getFloat32(8,true);
  let min2 = val.getFloat32(12,true);
  let min1 = val.getFloat32(16,true);
  log(min5.toFixed(2));
  log(min4.toFixed(2));
  log(min3.toFixed(2));
  log(min2.toFixed(2));
  log(min1.toFixed(2));
}

async function getMaxLimits(){
  let val = await chrMaxPressureLimits.readValue(); //returns a dataView
  let max5 = val.getFloat32(0,true);
  let max4 = val.getFloat32(4,true);
  let max3 = val.getFloat32(8,true);
  let max2 = val.getFloat32(12,true);
  let max1 = val.getFloat32(16,true);
  log(max5.toFixed(2));
  log(max4.toFixed(2));
  log(max3.toFixed(2));
  log(max2.toFixed(2));
  log(max1.toFixed(2));
}


//Preset the limits in the FlowIO and see if you can just read them. You would have to
//parse the 'val' variable in a different way.
//I can do this by using the offeset feature .getFloat32(4,true);
//The mode difficult question is about writing the 20 bytes to the characteristic.
//I shoud use an array buffer which is not limited to a specific size.

//##################################################################################

async function requestNewNumber(){
    let value = new Uint8Array([1]);
    await chrPressureRequest.writeValue(value);
}

function log(text) {
    console.log(text);
    document.querySelector('#log').textContent += text + '\n';
}

$(function updateSlider(){
    $("#slider-range").slider({
      range: true, min: 0, max: 500, values: [75, 300], step: 0.01, orientation: "horizontal",
      slide: (event, ui) => $("#label").val(ui.values[0] + " - " + ui.values[1])
    });
    $("#label").val($("#slider-range").slider("values", 0) + " - " + $("#slider-range").slider("values", 1));
  });
