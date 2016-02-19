var webSock = new WebSocket("ws://192.168.1.5/keysocket");

var ip = $('#ip-addr').on('keyup', function(){
        $('#ip').html($('#ip-addr').val());
      });

var direction = 0;
var keys = [false, false, false, false];

//changes ip adresses when text field changes
//
//

document.onkeydown = setKeyDown;
document.onkeyup = setKeyUp;

function check() {

console.log("keys:" + JSON.stringify(direction));

webSock.send("keys:" + JSON.stringify(direction));

}

function reset() {
direction = 0;
}

//---------------------------------------------
// setKeyDown
// Set the arrow key as down
//---------------------------------------------
function setKeyDown(e) {
    e = e || window.event;
    if (e.keyCode >= 37 || e.keyCode <= 40) {
		keys[e.keyCode - 37] = true;
    }
	sendData();
}

//---------------------------------------------
// setKeyDown
// Set the arrow key as up
//---------------------------------------------
function setKeyUp(e) {
    e = e || window.event;
    if (e.keyCode >= 37 || e.keyCode <= 40) {
		keys[e.keyCode - 37] = false;
    }
	sendData();
}

//---------------------------------------------
// sendData
// Send the key data over the websocket
//---------------------------------------------
function sendData(){
	direction = 0;
	if (keys[0]){direction += 4;} // Left
	if (keys[1]){direction += 1;} // Up
	if (keys[2]){direction += 8;} // Right
	if (keys[3]){direction += 2;} // Down
	console.log("keys:" + direction.toString());
	webSock.send("keys:" + direction.toString());
}
