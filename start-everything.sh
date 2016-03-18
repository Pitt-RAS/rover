#!/bin/bash

PID1=""
PID2=""

function get_pid {
PID1=`pidof python /home/pi/rover/server/server.py`
PID2=`pidof mjpg_streamer`
}

function stop {
get_pid
if [ -z $PID1  ] || [ -z $PID2  ]; then
    echo "server is not running."
    exit 1
else
    echo -n "Halting server..."
    kill -9 $PID1
    kill -9 $PID2
    sleep 1
    echo "... Done."
fi
}

function start {
get_pid
if [ -z $PID1  ] || [ -z $PID2  ]; then
    echo  "Starting server.."
###############################################################
    #cd "$(dirname "$0")"
    cd #mjpg-streamer
    cd /home/pi/rover/mjpg-streamer
    COUNTER=-1

    while [ $? != 0 ] || [ ${COUNTER} == "-1" ]; do
        ((COUNTER++))
        ./mjpg-streamer.sh stop /dev/video${COUNTER}
        ./mjpg-streamer.sh start /dev/video${COUNTER}
    done

    uvcdynctrl --set="Focus, Auto" 0

    #cd ../server
    python /home/pi/rover/server/server.py &
##############################################################
    get_pid
    echo "Done, PID(server.py)=$PID1 , PID(mjpeg-streamer)=$PID2"
else
    echo "server is already running, PID(server.py)=$PID1 , PID(mjpeg-streamer)=$PID2"
fi

}
function restart {
echo  "Restarting server.."
get_pid
if [ -z $PID1  ] && [ -z $PID2  ];then
    start
else
    stop
    sleep 5
    start
fi

}


function status {
get_pid
if [ -z $PID1 ] || [ -z $PID2 ]; then
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
