import tornado.ioloop
import tornado.web
import tornado.websocket
from tornado.websocket import websocket_connect
import json
import os
import serial
import struct
import time
import threading
import arduino_com

import time
from threading import Timer

from collections import deque

# Threads currently open in this application
open_threads = []
# Messages needed to be processed
messages = deque()


#-----------------------------------------------------
# cleanup
# Shut everything down
#-----------------------------------------------------
def cleanup():
    PWM.cleanup()
    GPIO.cleanup()
    for thread in open_threads:
        thread.stop()
        thread.join()

ping_sensor_distances = {'fl': -1, 'fr': -1, 'l': -1, 'r': -1, 'bl': -1, 'br': -1, 'b': -1}
def update_ping_sensors():
    #print(ping_sensor_distances)
    #arduino_com.read_ping_sensors(ping_sensor_distances)
    #print('done pinging')
    time.sleep(0.1)

#-----------------------------------------------------
# poll
# Poll the stopable thread
#-----------------------------------------------------
def poll(ws):
    while True:
        # Can't do anything if the websocket closed or the thread
        # has already been stopped
        if ws._closed or threading.current_thread().stopped():
            print('socket was closed')
            break
        #ws.write_message(u'send_input')
        battery = arduino_com.read_battery()
        update_ping_sensors()
        ws.write_message('{"type":"battery", "data":%f}'%battery);
        ws.write_message('{"type": "ping_sensors", "data": %s}'%json.dumps(ping_sensor_distances))
        time.sleep(polling_time)
    # Thread is dead, remove
    open_threads.remove(threading.current_thread())

#=========================================================
# StoppableThread
#---------------------------------------------------------
# The thread kept alive in the background
#=========================================================
class StoppableThread(threading.Thread):

    #-----------------------------------------------------
    # __init__
    # Start a new stoppable thread
    #-----------------------------------------------------
    def __init__(self, *args, **kwargs):
        super(StoppableThread, self).__init__(*args, **kwargs)
        self._stop = threading.Event()

    #-----------------------------------------------------
    # stop
    # Stop the thread
    #-----------------------------------------------------
    def stop(self):
        self._stop.set()

    #-----------------------------------------------------
    # stopped
    # Check if the thread is stopped
    #-----------------------------------------------------
    def stopped(self):
        return self._stop.isSet()

#=========================================================
# KeyPressHandler
#---------------------------------------------------------
# Handles all of the input from the web app
#=========================================================
class KeyPressHandler(tornado.websocket.WebSocketHandler):

    throttle = 1.0    # The speed multiplier for the entire robot's motors
    servoPos1 = 0        # The old position of the first servo
    servoPos2 = 0        # The old position of the second servo

    #-----------------------------------------------------
    # open
    # Opens the websocket, connecting to the app
    #-----------------------------------------------------
    def open(self):
        self._closed = False
        #print('Websocket Opened')
        new_thread = StoppableThread(target = poll, args = (self,))
        open_threads.append(new_thread)
        new_thread.start()

    #-----------------------------------------------------
    # on_message
    # Handles input from the app
    #-----------------------------------------------------
    def on_message(self, message):
        #print("received message")
        msg = json.loads(message)
        # Arrow key input for motors
        if (msg.has_key('Keys')):
            # Arrow keys are sent in a binary format:
            # 1 - Up, 2 - Down, 4 - Left, 8 - Right
            arrows = msg['Keys']
            direction = [arrows & 1, (arrows & 2) >> 1, (arrows & 4) >> 2, (arrows & 8) >> 3]
            wheels = [-direction[0]+direction[1]+direction[2]-direction[3],direction[0]-direction[1]+direction[2]-direction[3]]
            # The left and right sets of wheels will always move in the same direction.
            # If a specific wheel needs to be addressed instead, use mfl or mbl
            # Write the values to the arduino
            # #print('writing some velocity or something')
            arduino_com.controlMotors(wheels, self.throttle)

        # Slider used to adjust throttle for all motors
        if (msg.has_key('Thr')):
            self.throttle = msg['Thr'] / 256.0
        # Tilt used to adjust camera position
        if (msg.has_key('Tilt')):
            orientation = msg['Tilt']
            arduino_com.v_servo_write(orientation[1])
            print orientation
            arduino_com.h_servo_write(orientation[0])
        #print('released lock')
    #-----------------------------------------------------
    # check_origin
    # Set to true to allow all cross-origin traffic
    #-----------------------------------------------------
    def check_origin(self, origin):
        return True

    #-----------------------------------------------------
    # on_close
    # Close the socket (ignore errors)
    #-----------------------------------------------------
    def on_close(self):
        self._closed = True

#-----------------------------------------------------
# Program starts here
#-----------------------------------------------------
if __name__ == '__main__':
    print('Entered main')
    # Start the websocket
    application = tornado.web.Application([
        (r'/keysocket', KeyPressHandler),
        (r'/(.*)', tornado.web.StaticFileHandler, { 'path': os.path.join(os.path.dirname(os.path.realpath(__file__)), 'www'), 'default_filename': 'index.html' })
    ], debug = True)
    # Set up connection to Arduino on the USB port
    # arduino_serial = serial.Serial('/dev/ttyACM0', 115200);
    
    # Time in between thread polling
    polling_time = 0.3

    #conn = websocket_connect('ws://aftersomemath.com:8888/rover', on_message_callback = data_received)
    print('Starting port')
    application.listen(8080)
    print('Started port')
    
    print('Started Arduino Com')
    arduino_com.init()
    
    try:
        tornado.ioloop.IOLoop.instance().start()
    except KeyboardInterrupt:
        cleanup()
    finally:
        cleanup()
