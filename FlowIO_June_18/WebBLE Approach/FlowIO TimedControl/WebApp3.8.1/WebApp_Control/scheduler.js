window.onload = function(){
  addRow(); //we are creating the firt row of the table dynamically on load.
};

function executeSequence(){
  //1. Access the schedule table.
  //2. Read the time, action, and selected ports for the first data row (which is the second
  //  row of the table, but row numbers start from 0, so it is row number "1" anyways.)
  //3. Send the prameters obtained in step 2 to the executeAction() function.
  //4. Repeat the last two steps until all rows of the table are read.
  let scheduleTable = document.getElementById("schedule");
  let rows=scheduleTable.rows.length; //because the title row is not a line item.

  //TODO: Use the querySelectorAll method instead of the ID approach to obtain the
  //values from the scheduling table. I have shown how to do this for the starttime
  //and action fields, and now I just need to do the same for the ports field.
  //Then I can completely eliminate any references to IDs, and I won't have to
  //use the bad hack of naming the last row of the table with ID of 0.
  //I have already added classes to each port number: .port1, .port2, etc.
  //

  let startTimes = document.querySelectorAll("#schedule .starttime"); //creates an array of all the times.
  console.log(startTimes[0].value);
  console.log(startTimes[1].value);
  let actions = document.querySelectorAll("#schedule .action"); //creates an array of all the times.
  console.log(actions[0].value);
  console.log(actions[1].value);

  for(let i=1; i<rows-1; i++){ //we don't want the last row. We do it separately after the loop.
    let time = document.getElementById(`time_${i}`).value;
    let action = document.getElementById(`action_${i}`).value;
    let portsByte = getPortsByte(i);
    executeAction(time,action,portsByte);
  }

  //now we execute the last row separately whose id's are having the ending 0
  let time = document.getElementById(`time_0`).value;
  let action = document.getElementById(`action_0`).value;
  let portsByte=getPortsByte(0);
  executeAction(time,action,portsByte);

}

function executeAction(time,action,portsByte){ //the time specifies AT WHICH TIME the action will begin, NOT the duration of the action.
  let commandArray = new Uint8Array(2);
  if(action=="+")        commandArray[0] = 0x2b; //'+'
  else if(action == "-") commandArray[0] = 0x2d; //'-'
  else if(action == "^") commandArray[0] = 0x5e; //'^'
  else                   commandArray[0] = 0x21; //'!'

  commandArray[1] = portsByte;
  setTimeout(async function(){await chrCommand.writeValue(commandArray);}, parseInt(time));
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
  row.insertCell(0).innerHTML = `<input class="starttime" id="time_${i}" type="number" min="0" max="20000" value="0" step="100"><span> ms</span>`;
  row.insertCell(1).innerHTML = `
                    <select class="action" id="action_${i}">
                      <option value="+">Start Inflation</option>
                      <option value="-">Start Vacuum</option>
                      <option value="^">Start Release</option>
                      <option value="!">Stop Action</option>
                    </select>
                    `;
  row.insertCell(2).innerHTML = `
                <select class="device" id="device_${i}" value="1" disabled>
                  <option value="1">FlowIO 1</option>
                  <option value="2">FlowIO 2</option>
                  <option value="3">FlowIO 3</option>
                  <option value="4">FlowIO 4</option>
                  <option value="5">FlowIO 5</option>
                </select>
                `;
  row.insertCell(3).innerHTML = `
              <div>
                <label for="port1_${i}">1</label>
                <input type="checkbox" class="port1" id="port1_${i}" value="1">
                <label for="port2_${i}">2</label>
                <input type="checkbox" class="port2" id="port2_${i}" value="2">
                <label for="port3_${i}">3</label>
                <input type="checkbox" class="port3" id="port3_${i}" value="3">
                <label for="port4_${i}">4</label>
                <input type="checkbox" class="port4" id="port4_${i}" value="4">
                <label for="port5_${i}">5</label>
                <input type="checkbox" class="port5" id="port5_${i}" value="5">
              </div>
              `;
}
