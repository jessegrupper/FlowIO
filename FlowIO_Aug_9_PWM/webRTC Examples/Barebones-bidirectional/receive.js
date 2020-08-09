
let lastPeerId = null;
let peer = null; // Own peer object
let peerId = null;
let conn = null;
let recvId = document.getElementById("receiver-id");
let message = document.getElementById("message");
let sendMessageBox = document.getElementById("sendMessageBox");

function initialize() {   // Create own peer object with connection to shared PeerJS server, and set up event callbalks.
    peer = new Peer(null, {debug: 2});
    peer.on('open', id => {
        console.log('ID: ' + id);
        recvId.innerHTML = "ID: " + id;
    });
    peer.on('connection', c => {
        // Allow only a single connection
        if (conn && conn.open) {
            c.on('open', function() {
                c.send("Already connected to another client");
                setTimeout(function() { c.close(); }, 500);
            });
            return;
        }
        conn = c;
        console.log("Connected to: " + conn.peer);
        ready();
    });
    peer.on('disconnected', function () {
        console.log('Connection lost. Please reconnect');
    });
    peer.on('close', function() {
        conn = null;
        console.log('Connection destroyed');
    });
    peer.on('error', function (err) {
        console.log(err);
        alert('' + err);
    });
};
function ready() { //triggered once a connection has been achieved.
    conn.on('data', data => { //callbacdk for when we receive data.
        message.innerHTML = "<br>" + data + message.innerHTML;
        // switch (data) {
        //     case 'Off':
        //         break;
        //     case 'Reset':
        //         break;
        //     default:
        //         break;
        // };
    });
    conn.on('close', () => {
        console.log("Connection reset. Awaiting connection...");
        conn = null;
    });
}
function clearMessageLog() {
    message.innerHTML = "";
}
function sendMessage() {
    if (conn && conn.open) {
        var msg = sendMessageBox.value;
        sendMessageBox.value = "";
        conn.send(msg);
        console.log("Sent: " + msg)
        message.innerHTML = "<br>" + msg + message.innerHTML;
    } else {
        console.log('Connection is closed');
    }
}

initialize();
