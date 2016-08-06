#!/bin/bash

PID1=""
PID2=""
PID3=""
BITRATE="1000000"
JANUS="127.0.0.1"

function get_pid {
    PID1=`pidof python /home/pi/rover/server/server.py`
    PID2=`pidof janus`
    PID3=`pidof gst-launch-1.0`
}

function stop {
get_pid
if [ -z "${PID1}" ]; then
   echo "web server is not running"
else
   echo "killing webserver"
   sudo kill -9 $PID1
fi

if [ -z "${PID2}" ]; then
   echo "Janus is not running"
else
   echo "Killing Janus"
   sudo kill -9 $PID2
fi

if [ -z "${PID3}" ]; then
   echo "gstreamer is not running"
else
   echo "killing gstreamer"
   sudo kill -9 $PID3
fi
}

function start {
get_pid
if [ -z "${PID1}" ] || [ -z "${PID2}" ] || [ -z "${PID3}" ] ; then

    echo "Starting Janus"
    /opt/janus/bin/janus -F /opt/janus/etc/janus

    echo "Starting Server"
    #cd ../server
    sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-port 8080
    python /home/pi/rover/server/server.py >> /home/pi/rover/server/server.log 2>&1 &    
   
    startcameras
   
    get_pid
    echo "Done, PID(server.py)=$PID1 , PID(janus)=$PID2, PID(gstreamer)=$PID3"
else
    echo "server is already running, PID(server.py)=$PID1 , PID(janus)=$PID2"
fi

}
function restart {

echo  "Restarting janus, server, and webcams"
get_pid
if [ -z "${PID1}"  ] && [ -z "${PID2}" ];then
    start
else
    stop
    sleep 5
    start
fi

}


function status {
get_pid
if [ -z "${PID1}" ] || [ -z "${PID2}" ] || [ -z "${PID3}" ] ; then
    echo "Server is not running."
    exit 1
else
    echo "Running, PID(server.py)=$PID1 , PID(janus)=$PID2, PID(gstreamer)=$PID3"
fi

}

function startcameras {
    echo "start cameras"
    
    get_pid
    sudo kill -9 $PID3
    
    gst-launch-1.0 v4l2src  device=/dev/video0 ! video/x-raw,width=640,height=480, framerate=30/1 ! omxh264enc control-rate=1 target-bitrate="$BITRATE" ! h264parse config-interval=1 ! rtph264pay pt=98 ! udpsink host=$JANUS port=8004 >> /home/pi/rover/server/gstreamer-video0.log 2>&1 &

    gst-launch-1.0 v4l2src  device=/dev/video1 ! video/x-raw,width=640,height=480, framerate=30/1 ! omxh264enc control-rate=1 target-bitrate="$BITRATE" ! h264parse config-interval=1 ! rtph264pay pt=98 ! udpsink host=$JANUS port=8005 >> /home/pi/rover/server/gstreamer-video1.log 2>&1 &
}

case "$1" in
    start)
        if [ -n "$2" ] ; then
            if [[ $2 =~ ^[0-9]+$ ]]; then
                BITRATE=$2
                echo "setting bitrate"
            else
                echo "If you're going to set a bitrate make it a valid number"
            fi
        fi
        if [ -n "$3" ] ; then
           echo "using global camera server"
           JANUS="aftersomemath.com"
        fi
        start
        ;;
    stop)
        stop
        ;;
    restart)
        if [ -n "$2" ] ; then
            if [[ $2 =~ ^[0-9]+$ ]]; then
                BITRATE=$2
                echo "setting bitrate"
            else
                echo "If you're going to set a bitrate make it a valid number"
            fi
        fi
        
        if [ -n "$3" ] ; then
           echo "using global camera server"
           JANUS="aftersomemath.com"
        fi
        restart
        ;;
    status)
        status
        ;;
    startcameras)
        if [ -n "$2" ] ; then
            if [[ $2 =~ ^[0-9]+$ ]]; then
                BITRATE=$2
                echo "setting bitrate"
            else
                echo "If you're going to set a bitrate make it a valid number"
            fi
        fi
        
        if [ -n "$3" ] ; then
           echo "using global camera server"
           JANUS="aftersomemath.com"
        fi
        startcameras
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|status|startcameras}"
esac
