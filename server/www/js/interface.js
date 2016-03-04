var webSock = new WebSocket("ws://192.168.1.5/keysocket");

//var ip = $('#ip-addr').on('keyup', function(){
 //       $('#ip').html($('#ip-addr').val());
  //    });
      
var direction = 0;
var arrowKeys = [false, false, false, false];
var orientation = [0,0,0,0,0];
var allowTilt = true;

document.onkeydown = setKeyDown;
document.onkeyup = setKeyUp;
window.addEventListener("deviceorientation", updateOrientation, true);

//---------------------------------------------
// setKeyDown
// Set the arrow key as down
//---------------------------------------------
function setKeyDown(e) {
    e = e || window.event;
    // Quick access for arrow arrowKeys
    if (e.keyCode >= 37 && e.keyCode <= 40) {
        arrowKeys[e.keyCode - 37] = true;
    }else{
        // Check for tilt, keeping dt in mind
        console.log(e.keyCode);
        switch (e.keyCode){
            case 87: // W
                // Increase Beta by 1
                increaseRotation(2,1,180);
                break;
            case 83: // S
                // Decrease Beta by 1
                increaseRotation(2,-1,180);
                break;
            case 65: // A
                // Increase Gamma by 1
                increaseRotation(3,1,90);
                break;
            case 68: // D
                // Decrease Gamma by 1
                increaseRotation(3,-1,90);
                break;
        }
    }
    sendData();
}

//---------------------------------------------
// increaseRotation
// Clamp the rotation to specific limits
//---------------------------------------------
function increaseRotation(v, amt, lim){
    orientation[v] += amt;
    // Prevent it from going over
    while (orientation[v] > lim){
        orientation[v] -= (lim * 2);
    }
    // Prevent it from going under
    while (orientation[v] < -lim){
        orientation[v] += (lim * 2);
    }
    // Stop tilt control
    allowTilt = false;
}
//---------------------------------------------
// setKeyDown
// Set the arrow key as up
//---------------------------------------------
function setKeyUp(e) {
    e = e || window.event;
    if (e.keyCode >= 37 || e.keyCode <= 40) {
        arrowKeys[e.keyCode - 37] = false;
    }
    sendData();
}

//---------------------------------------------
// updateOrientation
// Send information about the orientation
//---------------------------------------------
function updateOrientation(e) {
  // Don't allow tilt if disabled
  if (!allowTilt){return;}
  var ab = Math.round(e.absolute);
  var a = Math.round(e.alpha);
  var b = Math.round(e.beta);
  var g = Math.round(e.gamma);
  var newOrientation = ab + a + b + g;
  // Check to see if we need to update anything
  if (Math.abs(newOrientation - orientation[4]) > 1){
    orientation = [ab, a, b, g, newOrientation];
    sendData();
  }
  
}

//---------------------------------------------
// sendData
// Send the key data over the websocket
//---------------------------------------------
function sendData(){
    direction = 0;
    if (arrowKeys[0]){direction += 4;} // Left
    if (arrowKeys[1]){direction += 1;} // Up
    if (arrowKeys[2]){direction += 8;} // Right
    if (arrowKeys[3]){direction += 2;} // Down
    var toSend = "{\"Keys\":"+direction.toString();
    toSend += ","+ "\"Tilt\":[";
    for (k = 0; k < 4; k++){
        toSend += orientation[k]+(k < 3 ? "," : "");
    }
    toSend += "]}";
    console.log(toSend);
    webSock.send(toSend);
}
