#!/bin/bash

PID1=""
PID2=""
PID3=""

function get_pid {
    PID1=`pidof python /home/pi/rover/server/server.py`
    PID2=`pidof janus`
    PID3='pidof gst-launch-1.0'
}

function stop {
get_pid
if [ -z "${PID1}" ]; then
   echo "web server is not running"
else
   echo "killing webserver"
   sudo kill -SIGKILL $PID1
fi

if [ -z "${PID2}" ]; then
   echo "Janus is not running"
else
   echo "Killing Janus"
   sudo kill -SIGKILL $PID2
fi

}

function start {
get_pid
if [ -z "${PID1}" ] || [ -z "${PID2}" ]; then
    echo "Turning off autofocus"
    uvcdynctrl -v -d video0 --set="Focus, Auto" 0
    uvcdynctrl -v -d video1 --set="Focus, Auto" 0

    echo "Starting Janus"
    /opt/janus/bin/janus -F /opt/janus/etc/janus

    #cd ../server
    sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-port 8080
    python /home/pi/rover/server/server.py >> /home/pi/rover/server/server.log 2>&1 &

    get_pid
    echo "Done, PID(server.py)=$PID1 , PID(janus)=$PID2"
else
    echo "server is already running, PID(server.py)=$PID1 , PID(janus)=$PID2"
fi

}
function restart {
echo  "Restarting server.."
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
if [ -z "${PID1}" ] || [ -z "${PID2}" ]; then
    echo "Server is not running."
    exit 1
else
    echo "Server is running, PID(server.py)=$PID1, PID(mjpeg-streamer)=$PID2"
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
