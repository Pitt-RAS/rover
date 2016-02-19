var webSock = new WebSocket("ws://192.168.1.5/keysocket");

var ip = $('#ip-addr').on('keyup', function(){
        $('#ip').html($('#ip-addr').val());
      });

var direction = 0;

//changes ip adresses when text field changes
//
//

document.onkeydown = checkKey;

function check() {

console.log("keys:" + JSON.stringify(direction));

webSock.send("keys:" + JSON.stringify(direction));

}

function reset() {
direction = 0;
}


function checkKey(e) {

    e = e || window.event;

    if (e.keyCode == '38') {
        // up arrow
        direction += 1;
        console.log("up");
    }
    if (e.keyCode == '40') {
        // down arrow
        direction += 2;
        console.log("down");
    }
    if (e.keyCode == '37') {
       // left arrow
        direction += 4;
        console.log("left");
    }
    if (e.keyCode == '39') {
       // right arrow
        direction += 8;
        console.log("right");
    }

console.log("keys:" + JSON.stringify(direction));

webSock.send("keys:" + JSON.stringify(direction));

direction = 0;
}
