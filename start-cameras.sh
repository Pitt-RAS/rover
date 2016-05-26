#!/bin/bash

PID1=""

function get_pid {
    PID1=`pidof gst-launch-1.0`
}

function stop {
get_pid
if [ -z "${PID1}" ]; then
   echo "gstreamer is not running"
else
   echo "killing gstreamer"
   sudo kill -SIGKILL $PID1
fi
}

function start {
get_pid
if [ -z "${PID1}" ] || [ -z "${PID2}" ]; then

    #Control rate 1 means variable bitrate

    echo "Starting Gstreamer"
    #gst-launch-1.0 v4l2src  device=/dev/video0 ! video/x-raw,width=640,height=480, framerate=30/1 ! omxh264enc control-rate=1 target-bitrate=700000 ! h264parse config-interval=1 ! rtph264pay pt=98 ! udpsink host=127.0.0.1 port=8004 >> /home/pi/rover/server/gstreamer-video0.log 2>&1 &

    #gst-launch-1.0 v4l2src  device=/dev/video1 ! video/x-raw,width=640,height=480, framerate=30/1 ! omxh264enc control-rate=1 target-bitrate=700000 ! h264parse config-interval=1 ! rtph264pay pt=98 ! udpsink host=127.0.0.1 port=8005 >> /home/pi/rover/server/gstreamer-video1.log 2>&1 &

    gst-launch-1.0 v4l2src  device=/dev/video0 ! video/x-raw,width=640,height=480, framerate=20/1 ! omxh264enc control-rate=1 target-bitrate=600000 ! h264parse config-interval=1 ! rtph264pay pt=98 ! udpsink host=aftersomemath.com port=8004 >> /home/pi/rover/server/gstreamer-video1.log 2>&1 &

    gst-launch-1.0 v4l2src  device=/dev/video1 ! video/x-raw,width=640,height=480, framerate=20/1 ! omxh264enc control-rate=1 target-bitrate=600000 ! h264parse config-interval=1 ! rtph264pay pt=98 ! udpsink host=aftersomemath.com port=8005 >> /home/pi/rover/server/gstreamer-video2.log 2>&1 &

    get_pid
    echo "Done, PID(gstreamer)=$PID1"
else
    echo "server is already running, PID(gstreamer)=$PID1"
fi

}
function restart {
echo  "Restarting gstreamer.."
get_pid
if [ -z "${PID1}" ];then
    start
else
    stop
    sleep 1
    start
fi

}


function status {
get_pid
if [ -z "${PID1}" ] ; then
    echo "Gstreamer is not running."
    exit 1
else
    echo "Gstreamer is running, PID(gstreamer.py)=$PID1"
fi

}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        restart
        ;;
    status)
        status
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|status}"
esac
