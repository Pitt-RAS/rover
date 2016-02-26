import tornado.ioloop
import tornado.web
import tornado.websocket
from tornado.websocket import websocket_connect

import RPIO as GPIO
import RPIO.PWM as PWM

import json
import serial
import time
import threading

from collections import deque

# Threads currently open in this application
open_threads = []

#-----------------------------------------------------
# set_camera_servo_position
# Sets the camera servo to a specified angle
#-----------------------------------------------------
#def set_camera_servo_position(servo, position):
	#pulse = camera_servo_pulse[servo] + float(camera_servo_pulse[servo+2] - camera_servo_pulse[servo]) * (position - camera_servo_angle[servo]) / (camera_servo_angle[servo+1] - camera_servo_angle[servo])
	#pulse = (pulse//10)*10
	#camera_servo1.set_servo(camera_servo_pins[servo], pulse)

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

#-----------------------------------------------------
# poll
# Poll the stopable thread
#-----------------------------------------------------
def poll(ws):
	while True:
		# Can't do anything if the websocket closed or the thread
		# has already been stopped
		if ws._closed or threading.current_thread().stopped():
			break
		#ws.write_message(u'send_input')
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

	throttle = 1.0	# The speed multiplier for the entire robot's motors

	#-----------------------------------------------------
	# open
	# Opens the websocket, connecting to the app
	#-----------------------------------------------------
	def open(self):
		self._closed = False
		print('Websocket Opened')
		new_thread = StoppableThread(target = poll, args = (self,))
		open_threads.append(new_thread)
		new_thread.start()

	#-----------------------------------------------------
	# on_message
	# Handles input from the app
	#-----------------------------------------------------
	def on_message(self, message):
		msg = json.loads(message)
		
		# Arrow key input for motors
		if (msg.has_key('Keys')):
			# Arrow keys are sent in a binary format:
			# 1 - Up, 2 - Down, 4 - Left, 8 - Right
			arrows = msg['Keys']
			dir = [arrows & 1, (arrows & 2) >> 1, (arrows & 4) >> 2, (arrows & 8) >> 3]
			wheels = [-dir[0]+dir[1]+dir[2]-dir[3],dir[0]-dir[1]+dir[2]-dir[3]]
			# The left and right sets of wheels will always move in the same direction.
			# If a specific wheel needs to be addressed instead, use mfl or mbl
			w0 = '%.7f'% (wheels[0]*self.throttle)
			w1 = '%.7f'% (wheels[1]*self.throttle)
			mal = 'mal'+w0[:7]+':'
			mar = 'mar'+w1[:7]+':'
			# Write the values to the arduino
			arduino_serial.write(mal)
			arduino_serial.write(mar+'\n')
					
		# Slider used to adjust throttle for all motors
		if (msg.has_key('Thr')):
			self.throttle = msg['Thr'] / 256.0

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
		
def data_received(message):
	if message != None:
		print("Message received: " + message)
		#Need to handle message, cannot handle hardware IO here as another incoming data piece will destroy this one and cause exceptions if it does not finish quickly
		#It will require another thread to handle the stream of input data
	else:
		print("Connection was closed")
	
#-----------------------------------------------------
# Program starts here
#-----------------------------------------------------
if __name__ == '__main__':
	# Start the websocket
	application = tornado.web.Application([
		(r'/keysocket', KeyPressHandler),
		(r'/(.*)', tornado.web.StaticFileHandler, { 'path': './www', 'default_filename': 'index.html' })
	])
	# Set up connection to Arduino on the USB port
	arduino_serial = serial.Serial('/dev/ttyAMA0', 115200);
	# Time in between thread polling
	polling_time = 0.1
	# Pins that the camera uses
	camera_servo_pins = [27,22]
	# Camera Servo Pulse = [min1, min2, max1, max2]
	camera_servo_pulse = [18950,18740,19650,19370]
	# Camera Servo Angle = [min1, min2, max1, max2]
	camera_servo_angle = [-75, -35, 75, 90]

	camera_servo1 = PWM.Servo(12)
	camera_servo2 = PWM.Servo(13)
	# Set camera servos 0 and 1 to 0
	#set_camera_servo_position(0,0)
	#set_camera_servo_position(1,0)


	conn = websocket_connect('ws://aftersomemath.com:8888/rover', on_message_callback = data_received)

	application.listen(80)
	try:
		tornado.ioloop.IOLoop.instance().start()
	except KeyboardInterrupt:
		cleanup()
	finally:
		cleanup()
