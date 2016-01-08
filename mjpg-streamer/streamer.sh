#!/bin/bash
killall mjpg_streamer
/usr/local/bin/mjpg_streamer -i "/usr/local/lib/input_uvc.so -n -f 10 -r 320x240" -o "/usr/local/lib/output_http.so -p 8090 -w /usr/local/www" 
