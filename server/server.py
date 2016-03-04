import tornado.ioloop
import tornado.web
import tornado.websocket
from tornado.websocket import websocket_connect

import RPIO as GPIO
import RPIO.PWM as PWM

import json
import serial
import struct
import time
import threading

from collections import deque

# Threads currently open in this application
open_threads = []
# Messages needed to be processed
messages = deque()

#-----------------------------------------------------
# set_camera_servo_position
# Sets the camera servo to a specified angle
#-----------------------------------------------------
#def set_camera_servo_position(servo, position):
	#pulse = camera_servo_pulse[servo] + float(camera_servo_pulse[servo+2] - camera_servo_pulse[servo]) * (position - camera_servo_angle[servo]) / (camera_servo_angle[servo+1] - camera_servo_angle[servo])
	#pulse = (pulse//10)*10
	#camera_servo1.set_servo(camera_servo_pins[servo], pulse)
def set_camera_servo1_position(position):
	print("updating servo1:" + str(position))
	pulse = camera_servo1_min_pulse + camera_servo1_pulse_diff * (position - camera_servo1_min_angle) / (camera_servo1_max_angle - camera_servo1_min_angle)
	pulse = (pulse//10)*10
	camera_servo1.set_servo(camera_servo1_pin, pulse)
	
def set_camera_servo2_position(position):
	print("updating servo2" + str(position))
	pulse = camera_servo2_min_pulse + camera_servo2_pulse_diff * (position - camera_servo2_min_angle) / (camera_servo2_max_angle - camera_servo2_min_angle)
	pulse = (pulse//10)*10
	camera_servo2.set_servo(camera_servo2_pin, pulse)

	
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
	servoPos1 = 0		# The old position of the first servo
	servoPos2 = 0		# The old position of the second servo

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
			direction = [arrows & 1, (arrows & 2) >> 1, (arrows & 4) >> 2, (arrows & 8) >> 3]
			wheels = [-direction[0]+direction[1]+direction[2]-direction[3],direction[0]-direction[1]+direction[2]-direction[3]]
			# The left and right sets of wheels will always move in the same direction.
			# If a specific wheel needs to be addressed instead, use mfl or mbl
			# Write the values to the arduino
			arduino_serial.write('mal')
                        for b in struct.pack('f', wheels[0] * self.throttle):
                            arduino_serial.write(b)
                        arduino_serial.write(':')
			arduino_serial.write('mar')
                        for b in struct.pack('f', wheels[1] * self.throttle):
                            arduino_serial.write(b)
                        arduino_serial.write(':')

		# Slider used to adjust throttle for all motors
		if (msg.has_key('Thr')):
			self.throttle = msg['Thr'] / 256.0
		# Tilt used to adjust camera position
		if (msg.has_key('Tilt')):
			orientation = msg['Tilt']
			if (abs(self.servoPos1 - orientation[2]) >= 5):
				self.servoPos1 = orientation[2]
				set_camera_servo1_position(orientation[2]/2)
			if (abs(self.servoPos2 - orientation[3]) >= 5):
				self.servoPos2 = orientation[3]
				set_camera_servo2_position(orientation[3])

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
		messages.enqueue(message)
		#Need to handle message, cannot handle hardware IO here as another incoming data piece will destroy this one and cause exceptions if it does not finish quickly
		#It will require another thread to handle the stream of input data
	else:
		print("Connection was closed")

		
def initialize_camera_servos():
	global camera_servo1_pin, camera_servo2_pin
	global camera_servo1_min_pulse, camera_servo2_min_pulse
	global camera_servo1_max_pulse, camera_servo2_max_pulse
	global camera_servo1_min_angle, camera_servo2_min_angle
	global camera_servo1_max_angle, camera_servo2_max_angle
	global camera_servo1, camera_servo2
	global camera_servo1_pulse_diff, camera_servo2_pulse_diff

	camera_servo1_pin = 27
	camera_servo2_pin = 22

	camera_servo1_min_pulse = 18950
	camera_servo1_max_pulse = 19650
	camera_servo2_min_pulse = 18740
	camera_servo2_max_pulse = 19370

	camera_servo1_min_angle = -75
	camera_servo1_max_angle = 75
	camera_servo2_min_angle = -35
	camera_servo2_max_angle = 90
	
	camera_servo1_pulse_diff = float(camera_servo1_max_pulse - camera_servo1_min_pulse)
	camera_servo2_pulse_diff = float(camera_servo2_max_pulse - camera_servo2_min_pulse)

	camera_servo1 = PWM.Servo(12)
	camera_servo2 = PWM.Servo(13)
	set_camera_servo1_position(0)
	set_camera_servo2_position(0)
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
	initialize_camera_servos()

	conn = websocket_connect('ws://aftersomemath.com:8888/rover', on_message_callback = data_received)

	application.listen(80)
	try:
		tornado.ioloop.IOLoop.instance().start()
	except KeyboardInterrupt:
		cleanup()
	finally:
		cleanup()
