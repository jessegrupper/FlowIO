
let lastPeerId = null;
let peer = null; // Own peer object
let peerId = null;
let conn = null;
let recvId = document.getElementById("receiver-id");
let messageLog = document.getElementById("messageLog");

function initialize() {   // Create own peer object with connection to shared PeerJS server, and set up event callbalks.
    //Default Server:
    //peer = new Peer(null, {debug: 2});

    //Custom Server:
    peer = new Peer({host:'peerjs-server.herokuapp.com', secure:true, port:443})

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
        messageLog.innerHTML = "<br>" + data + messageLog.innerHTML;
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
    messageLog.innerHTML = "";
}

initialize();
