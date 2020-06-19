/*
  An earlier version of this file was based on dynamic ID generation, for which
  we used the syntax `idNmae_${i}`. Whenever you put a string inside of backticks
  you can insert variables into the string as shown.
*/

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

  let startTimes = document.querySelectorAll("#schedule .starttime"); //creates an array of all the times.
  let actions = document.querySelectorAll("#schedule .action"); //creates an array of all the actions.
  let port1 = document.querySelectorAll("#schedule .port1"); //creates an array of all port1 states.
  let port2 = document.querySelectorAll("#schedule .port2"); //creates an array of all port2 states.
  let port3 = document.querySelectorAll("#schedule .port3"); //creates an array of all port3 states.
  let port4 = document.querySelectorAll("#schedule .port4"); //creates an array of all port4 states.
  let port5 = document.querySelectorAll("#schedule .port5"); //creates an array of all port5 states.

  for(let i=0; i<startTimes.length; i++){
    let p1=port1[i].checked;
    let p2=port2[i].checked;
    let p3=port3[i].checked;
    let p4=port4[i].checked;
    let p5=port5[i].checked;

    let portsByte = getPortsByte(p1,p2,p3,p4,p5); //each input is either true or false.
    executeAction(startTimes[i].value, actions[i].value, portsByte)
  }
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

function getPortsByte(p1,p2,p3,p4,p5){
  let portsByte = 0x00;
  if(p1) portsByte ^= 0x01; //0 0001
  if(p2) portsByte ^= 0x02; //0 0010
  if(p3) portsByte ^= 0x04; //0 0100
  if(p4) portsByte ^= 0x08; //0 1000
  if(p5) portsByte ^= 0x10; //1 0000
  return portsByte;
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
  row.insertCell(0).innerHTML = `<input class="starttime" type="number" min="0" max="20000" value="0" step="100"><span> ms</span>`;
  row.insertCell(1).innerHTML = `
                    <select class="action">
                      <option value="+">Start Inflation</option>
                      <option value="-">Start Vacuum</option>
                      <option value="^">Start Release</option>
                      <option value="!">Stop Action</option>
                    </select>
                    `;
  row.insertCell(2).innerHTML = `
                <select class="device" value="1" disabled>
                  <option value="1">FlowIO 1</option>
                  <option value="2">FlowIO 2</option>
                  <option value="3">FlowIO 3</option>
                  <option value="4">FlowIO 4</option>
                  <option value="5">FlowIO 5</option>
                </select>
                `;
  row.insertCell(3).innerHTML = `
              <div>
                <label>1</label>
                <input type="checkbox" class="port1" value="1">
                <label>2</label>
                <input type="checkbox" class="port2" value="2">
                <label>3</label>
                <input type="checkbox" class="port3" value="3">
                <label>4</label>
                <input type="checkbox" class="port4" value="4">
                <label>5</label>
                <input type="checkbox" class="port5" value="5">
              </div>
              `;
}
