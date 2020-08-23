let peer = null; // own peer object
let conn = null;

let recvIdInput = document.getElementById("receiver-id");
let constatus = document.getElementById("constatus");
let msgLog = document.getElementById("msgLog");

function initialize() { //creates a peer object for our end, and sets up callbacks.
    // Create own peer object with connection to shared PeerJS server

    //Default Server:
    //peer = new Peer(null, {debug: 2});

    //Custom Server:
    peer = new Peer({host:'peerjs-server.herokuapp.com', secure:true, port:443})

    peer.on('disconnected', function () {
        constatus.innerHTML = "Connection lost. Please reconnect";
        console.log('Connection lost. Please reconnect');
    });
    peer.on('close', function() {
        conn = null;
        constatus.innerHTML = "Connection destroyed. Please refresh";
        console.log('Connection destroyed');
    });
    peer.on('error', function (err) {
        console.log(err);
        alert('' + err);
    });
};

function join() { //create a new connection between two peers, and set up event callbacks.
    if(conn) conn.close(); //close existing connection.
    conn = peer.connect(recvIdInput.value, {reliable: true}); //create a new connection
                    //to the specified peer ID. Set the mode to reliable.
    conn.on('open', function () {
        constatus.innerHTML = "Connected to: " + conn.peer;
        console.log("Connected to: " + conn.peer);
    });
    conn.on('data', function (data) { //when we receive some msg from the peer.
        msgLog.innerHTML = "<br> Peer:" + data + msgLog.innerHTML;
    });
    conn.on('close', function () {
        constatus.innerHTML = "Connection closed";
    });
};

function signal(sigName) { //The argument of this function gets sent to the peer.
    if(conn && conn.open) {
        conn.send(sigName);
        console.log(sigName + " signal sent");
        msgLog.innerHTML = "<br>" + sigName + msgLog.innerHTML;
    }
    else console.log('Connection is closed');
}

function clearMessageLog() {
  msgLog.innerHTML = "";
};

initialize(); //start the process of obtaining an ID
