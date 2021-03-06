#!/bin/bash

PID1=""
PID2=""

function get_pid {
    PID1=`pidof python /home/pi/rover/server/server.py`
    PID2=`pidof mjpg_streamer`
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
    echo "cameras are not running."
else
    echo "halting cameras"
    cd /home/pi/rover/mjpg-streamer
    COUNTER=-1

    while [ $? != 0 ] || [ ${COUNTER} == "-1" ]; do
        ((COUNTER++))
        ./mjpg-streamer.sh stop /dev/video${COUNTER} 8081
    done

    ((COUNTER++))
    ./mjpg-streamer.sh stop /dev/video${COUNTER} 8082 
   
    while [ $? != 0 ]; do
        ((COUNTER++))
        ./mjpg-streamer.sh stop /dev/video${COUNTER} 8082
    done

    sleep 1
    echo "... Done."
fi
}

function start {
get_pid
if [ -z "${PID1}" ] || [ -z "${PID2}" ]; then
    echo  "Starting server.."
###############################################################
    #cd "$(dirname "$0")"
    cd #mjpg-streamer
    cd /home/pi/rover/mjpg-streamer
    COUNTER=-1

    #while [ $? != 0 ] || [ ${COUNTER} == "-1" ]; do
    #    ((COUNTER++))
    #    ./mjpg-streamer.sh stop /dev/video${COUNTER} 8081
    #    ./mjpg-streamer.sh start /dev/video${COUNTER} 8081
    #done

    #uvcdynctrl -v -d video0 --set="Focus, Auto" 0
    #uvcdynctrl -v -d video${COUNTER} --set="Focus, Auto" 0

    #((COUNTER++))
    #./mjpg-streamer.sh stop /dev/video${COUNTER} 8082 
    #./mjpg-streamer.sh start /dev/video${COUNTER} 8082
    #   
    #while [ $? != 0 ]; do
    #    ((COUNTER++))
    #    ./mjpg-streamer.sh stop /dev/video${COUNTER} 8082
    #    ./mjpg-streamer.sh start /dev/video${COUNTER} 8082
    #done

    #uvcdynctrl -v -d video1 --set="Focus, Auto" 0
    #uvcdynctrl -v -d video${COUNTER} --set="Focus, Auto" 0
    echo "/dev/video0"
    ./mjpg-streamer.sh stop /dev/video0 8081
    ./mjpg-streamer.sh start /dev/video0 8081
    echo "/dev/video1"
    ./mjpg-streamer.sh stop /dev/video1 8082
    ./mjpg-streamer.sh start /dev/video1 8082
    uvcdynctrl -v -d video0 --set="Focus, Auto" 0
    uvcdynctrl -v -d video1 --set="Focus, Auto" 0
    #cd ../server
    sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-port 8080
    python /home/pi/rover/server/server.py >> /home/pi/rover/server/server.log 2>&1 &
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
