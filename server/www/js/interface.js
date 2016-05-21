var CamMotion = {
  UP: false,
  DOWN: false,
  LEFT: false,
  RIGHT: false,
  vertical: 0,
  horizontal: 0,
  rotationSpeed: (150.0/1000.0) * 50.0, // 80 degrees per second / (S/mS) * (50mS)
  verticalMax: 80,
  verticalMin: -50,
  horizontalMax: 70, //Only Need one horizontal max because rotation is equal on the left and right
};

var RobotMotion = {
    FORWARD_V: 0,
    ROTATION_V: 0,
}

var tiltDot;

var timeOfLastMessage = 0;

$(document).ready(function() {
    var webSock = new WebSocket("ws://192.168.2.115/keysocket");
    webSock.onmessage = getData;
  
    if($('#SwapEyes').length > 0){
        document.getElementById ("SwapEyes").addEventListener ("click", swapEyes, false);
    }
    
    if ($('#TiltDot').length > 0) {
            tiltDot = document.getElementById('TiltDot');
            console.log(tiltDot);
    }
    
    document.onkeydown = setKeyDown;
    document.onkeyup = setKeyUp;
    
    // If on mobile
    if( /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) )
    {
        window.addEventListener("deviceorientation", updateOrientation, true);
        
        //Go full screen when clicked
        addEventListener("click", function() {
            var el = document.documentElement
            , rfs = el.requestFullScreen || el.webkitRequestFullScreen || el.mozRequestFullScreen;
            rfs.call(el);
            });
        }
    
    $('#fullscreen-button').click(toggleFullscreen);
    
    setInterval(function(){ cameraUpdateOrientation(); }, 50);

    //---------------------------------------------
    // Update Camera Orientation variables
    //---------------------------------------------
    function cameraUpdateOrientation()
    {
      if(CamMotion.UP || CamMotion.DOWN || CamMotion.RIGHT || CamMotion.LEFT)
      {
        if(CamMotion.UP)
        {
            CamMotion.vertical += CamMotion.rotationSpeed;
            CamMotion.vertical = CamMotion.vertical > CamMotion.verticalMax ? CamMotion.verticalMax : CamMotion.vertical;
        }
        if(CamMotion.DOWN)
        {
            CamMotion.vertical -= CamMotion.rotationSpeed;;
            CamMotion.vertical = CamMotion.vertical < CamMotion.verticalMin ? CamMotion.verticalMin : CamMotion.vertical;
        }
        if(CamMotion.RIGHT)
        {
            CamMotion.horizontal += CamMotion.rotationSpeed;;
            CamMotion.horizontal = CamMotion.horizontal > CamMotion.horizontalMax ? CamMotion.horizontalMax : CamMotion.horizontal;
        }
        if(CamMotion.LEFT)
        {
            CamMotion.horizontal -= CamMotion.rotationSpeed;;
            CamMotion.horizontal = -CamMotion.horizontal > CamMotion.horizontalMax ? -CamMotion.horizontalMax : CamMotion.horizontal;
        }
        // Update the tilt display
        updateTiltDot();
        // Send new info only if the last packet hasn't been sent, helps with slow connections
        safeSendData();
      }
    }
    //---------------------------------------------
    // getData
    // Gets a message over the websocket
    //---------------------------------------------
    function getData (event) {
        var msg = JSON.parse(event.data);
        console.log(msg);
        // We can select specific JSON groups by using msg.name, where JSON contains "name":x
        // Every type MUST have msg.type to determine what else is pulled from it
        switch (msg.type){
            case "print": // Print out msg.data
                console.log(msg.data);
                break;
            case "battery":
                $('#battery-voltage').text(msg.data);
                break;
            case "ping_sensors":
                $('#ping-display').text(JSON.stringify(msg.data));
                break;
        }
    }

    //---------------------------------------------
    // setKeyDown
    // Only camera orientation events currently
    //---------------------------------------------
    function setKeyDown(e) {
        e = e || window.event;
        switch (e.keyCode){
            // Robot motion codes
            case 37: // Left Arrow
                RobotMotion.ROTATION_V = -100;
                break;
            case 38: // Up Arrow
                RobotMotion.FORWARD_V = 100;
                break;
            case 39: // Right Arrow
                RobotMotion.ROTATION_V = 100;
                break;
            case 40: // Down Arrow
                RobotMotion.FORWARD_V = -100;
                break;
                
            // Camera Rotation Codes
            case 87: // W
                CamMotion.UP = true;
                break;
            case 83: // S
                CamMotion.DOWN = true;
                break;
            case 65: // A
                CamMotion.LEFT = true;
                break;
            case 68: // D
                CamMotion.RIGHT = true;
                break;
        }
        
        // Send immediately because we have to send motor events
        safeSendData();
    }
    
    //---------------------------------------------
    // setKeyDown
    // Set the arrow keys as up
    //---------------------------------------------
    function setKeyUp(e) {
        switch (e.keyCode){
            // Robot motion codes
            case 37: // Left Arrow
                RobotMotion.ROTATION_V = 0;
                break;
            case 38: // Up Arrow
                RobotMotion.FORWARD_V = 0;
                break;
            case 39: // Right Arrow
                RobotMotion.ROTATION_V = 0;
                break;
            case 40: // Down Arrow
                RobotMotion.FORWARD_V = 0;
                break;
                
            // Camera Rotation Codes
            case 87: // W
                CamMotion.UP = false;
                break;
            case 83: // S
                CamMotion.DOWN = false;
                break;
            case 65: // A
                CamMotion.LEFT = false;
                break;
            case 68: // D
                CamMotion.RIGHT = false;
                break;
        }
        // Send immediately because we have to send motor events
        sendData();
    }
    
    //---------------------------------------------
    // updateOrientation
    // Send information about the orientation
    //---------------------------------------------
    function updateOrientation(e) {
        var a = Math.round(e.alpha); // Left and Right
        var g = Math.round(e.gamma);// Up and down
        // The below rules for fixing gamma and alpha were found by watching initial values and playing with the phone
        // Fix gamma so it doesn't jump
        if(g < 0)
        {
            g+=180;
        }

        g -= 90;
        g = g > CamMotion.verticalMax ? CamMotion.verticalMax : g;
        g = g < CamMotion.verticalMin ? CamMotion.verticalMin : g;
        
        // Fix alpha so it doesn't jump
        // There are different rules if gamma is more than or less than 0
        if(g > 0)
        {
            a -= 180; 
        }
        else
        {
            if(a > 180)
            {
                a -= 360;
            }
        }
        a = a > CamMotion.horizontalMax ? CamMotion.horizontalMax : a;
        a = a < -CamMotion.horizontalMax ? -CamMotion.horizontalMax : a;
        
        // This may be useful for debugging other phones someday so leaving it here
        //$('#rotAlpha').text(a);
        //$('#rotBeta').text(b);
        //$('#rotGamma').text(g);
        
        CamMotion.vertical = g;
        CamMotion.horizontal = a;

        // Update the tilt display
        updateTiltDot();
        
        // Safely send the new info
        safeSendData();
    }
    
    //---------------------------------------------
    // updateTiltDot
    // Updates the position of the tilt dot
    //---------------------------------------------
    function updateTiltDot(){
        if( tiltDot != null)
        {
            console.log("updating");
            tiltDot.style.left = (CamMotion.horizontal * (57 / 90)) + 'px';
            tiltDot.style.top = (-CamMotion.vertical * (57 / 180)) + 'px';
            $('#servo-vertical-angle').text(CamMotion.vertical);
            $('#servo-horizontal-angle').text(CamMotion.horizontal);
        }
    }

    
    //---------------------------------------------
    // Only send data if the last packet went through
    // Do not use this if your data must be sent! (Aka motor events)
    // Only if it should not be sent when something else is sending
    //---------------------------------------------
    function safeSendData(){
      
         var date = new Date();
        if(webSock.bufferedAmount == 0 && ((date.getTime() - timeOfLastMessage) > 100))
        {
            timeOfLastMessage = date.getTime();
            sendData();
        }
    }
    
    //---------------------------------------------
    // sendData
    // Send the key data over the websocket
    //---------------------------------------------
    function sendData(){
        var toSend = "{\"Velocity\":["
        toSend += RobotMotion.FORWARD_V + "," + RobotMotion.ROTATION_V + "],";
        toSend += "\"Tilt\":[";
        toSend += CamMotion.horizontal + "," + CamMotion.vertical + "]}";
        console.log(toSend);
        webSock.send(toSend);
    }

    //---------------------------------------------
    // swapEyes
    // Switches the left eye stream with the right
    //---------------------------------------------
    function swapEyes()
    {
        var eyer = document.getElementById('EyeR');
        var eyel = document.getElementById('EyeL');
        var eyer_src = eyer.src;
        eyer.src = eyel.src;
        eyel.src = eyer_src;
    }

    //----------------------------------------------
    // toggleFullscreen
    //----------------------------------------------
    function toggleFullscreen(e) {
        var elem = $('.interface-center').get()[0];
        if (!document.fullscreenElement &&    // alternative standard method
            !document.mozFullScreenElement &&
            !document.webkitFullscreenElement &&
            !document.msFullscreenElement ) {  // current working methods

            if (elem.requestFullscreen) {
                elem.requestFullscreen();
            } else if (elem.msRequestFullscreen) {
                elem.msRequestFullscreen();
            } else if (elem.mozRequestFullScreen) {
                elem.mozRequestFullScreen();
            } else if (elem.webkitRequestFullscreen) {
                elem.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
            }
        } else {
            if (document.exitFullscreen) {
                document.exitFullscreen();
            } else if (document.msExitFullscreen) {
                document.msExitFullscreen();
            } else if (document.mozCancelFullScreen) {
                document.mozCancelFullScreen();
            } else if (document.webkitExitFullscreen) {
                document.webkitExitFullscreen();
            }
        }
    }
});
