$(document).ready(function() {
    var webSock = new WebSocket("ws://192.168.2.115/keysocket");
    webSock.onmessage = getData;

    //var ip = $('#ip-addr').on('keyup', function(){
     //       $('#ip').html($('#ip-addr').val());
      //    });
	  
	document.getElementById ("SwapEyes").addEventListener ("click", swapEyes, false);
	document.getElementById ("RecalibrateTilt").addEventListener("click", function(){needRecalibrateTilt = true;}, false);
          
    var direction = 0;
    var arrowKeys = [false, false, false, false];
    var orientation = [0,0,0,0,0,0];
	var tiltCalibration = [0,0,0,0];
    var allowTilt = false;
	var needRecalibrateTilt = false;
    var rotationSpeed = 10
    
    var tiltDot = null;
    var tiltRadar = null;
    var tiltRadarX = 0;
    var tiltRadarY = 0;
    
    document.onkeydown = setKeyDown;
    document.onkeyup = setKeyUp;
    window.addEventListener("deviceorientation", updateOrientation, true);
    $('#fullscreen-button').click(toggleFullscreen);
	
	// Calibrate the tilt after half a second
	setTimeout(function(){needRecalibrateTilt = true;}, 100);
    
    //---------------------------------------------
    // getData
    // Gets a message over the websocket
    //---------------------------------------------
    function getData (event) {
        //console.log(event.data);
        var msg = JSON.parse(event.data);
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
    // Set the arrow key as down
    //---------------------------------------------
    function setKeyDown(e) {
        e = e || window.event;
        // Quick access for arrow arrowKeys
        if (e.keyCode >= 37 && e.keyCode <= 40) {
            arrowKeys[e.keyCode - 37] = true;
        }else{
            // Check for tilt, keeping dt in mind
            switch (e.keyCode){
                case 87: // W
                    // Increase Gamma by 1
                    increaseRotation(2,rotationSpeed,90);
                    break;
                case 83: // S
                    // Decrease Gamma by 1
                    increaseRotation(2,-rotationSpeed,90);
                    break;
                case 65: // A
                    // Increase Alpha by 1
                    increaseRotation(3,rotationSpeed,180);
                    break;
                case 68: // D
                    // Decrease Alpha by 1
                    increaseRotation(3,-rotationSpeed,180);
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
            orientation[v] = lim;
        }
        // Prevent it from going under
        while (orientation[v] < -lim){
            orientation[v] = -lim;
        }
        // Stop tilt control
        allowTilt = false;
    }
	//---------------------------------------------
    // setRotation
    // Clamp the rotation to specific limits
    //---------------------------------------------
    function setRotation(v, amt, lim){
        orientation[v] = amt;
        // Prevent it from going over
        if (orientation[v] > lim){
            orientation[v] = lim;
        }
        // Prevent it from going under
        if (orientation[v] < -lim){
            orientation[v] = -lim;
        }
		return orientation[v];
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
	  // Check if we need to recalibrate
	  if (needRecalibrateTilt){
	  console.log("Try calibrate");
		needRecalibrateTilt = false;
		recalibrateTilt(e);
	  }
      // Don't allow tilt if disabled
      if (!allowTilt){return;}
      var ab = Math.round(e.absolute);
      var a = Math.round(e.alpha); // Left and Right
      var b = Math.round(e.beta);
      var g = Math.round(e.gamma);// Up and down
	  // Normalize all values using the calibration values
	  //g = setRotation(2,g - tiltCalibration[3],90);
	  // Fix gamma so it doesn't jump
	  var inv_g = (g < 0 ? 180 : 0);
	  //if (g > 0){
	  //  g -= 90;
	  //}else{
	  //  g += 90;
	  //}
	  //g -= (tiltCalibration[3] + 90);
	  //if (g < -90){
	  //  g  += 180;
	  //}else if (g > 90) {
	  //  g -= 180;
	  //}
      if (g <= 0) {
          g = 90 - g;
      } else {
          g = -90 - g;
      }
      g -= tiltCalibration[3];
	  //a = setRotation(3,a - tiltCalibration[1],180);
	  a -= (tiltCalibration[1]/* + inv_g*/);
	  //if (a < 0){
	  //  a += 360;
	  //}
	  b = 0;
	  ab = 0;
	  $('#rotAlpha').text(a);
	  $('#rotBeta').text(b);
	  $('#rotGamma').text(g);
      var newOrientation = ab + a + b + g;
      // Check to see if we need to update anything
      if (Math.abs(newOrientation - orientation[4]) > 1){
        orientation = [ab, a, g, b, newOrientation];
        sendData();
      }
      
    }
	
	function recalibrateTilt(e){
      var ab = Math.round(e.absolute);
      var a = Math.round(e.alpha); // Left and Right
      var b = Math.round(e.beta);
      var g = Math.round(e.gamma); // Up and down
      if (g >= 0) {
          g = 90 - g;
      } else {
          g = -90 - g;
      }
	  tiltCalibration = [ab, a, b, g];
	  console.log("Calibrated");
	  allowTilt = true;
	}
    
    //---------------------------------------------
    // updateTiltDot
    // Updates the position of the tilt dot
    //---------------------------------------------
    function updateTiltDot(){
        if (tiltDot == null || tiltRadar == null){
    		tiltDot = document.getElementById('TiltDot');
            //tiltDot.style.position = 'relative'; 
    		tiltRadar = document.getElementById('TiltDot');
    		//tiltRadar.style.position = 'relative';
    		
    		//tiltRadarX = parseInt(tiltRadar.style.left);
    		//tiltRadarY = parseInt(tiltRadar.style.top);
    	}
    	tiltDot.style.left = (orientation[3] * (57 / 90)) + 'px';
    	tiltDot.style.top = (orientation[2] * (57 / 180)) + 'px';
    	//console.log(tiltDot.style.left);
        $('#servo-vertical-angle').text(orientation[2]);
        $('#servo-horizontal-angle').text(orientation[3]);
    }

    //---------------------------------------------
    // sendData
    // Send the key data over the websocket
    //---------------------------------------------
    function sendData(){
    	// Update the tilt display
    	updateTiltDot();
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

	//---------------------------------------------
    // swapEyes
    // Switches the left eye stream with the right
    //---------------------------------------------
	function swapEyes(){
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
