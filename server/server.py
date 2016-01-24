import tornado.ioloop
import tornado.web
import tornado.websocket

import RPIO as GPIO
import RPIO.PWM as PWM

import json
import serial
import time
import threading

from collections import deque

def set_camera_servo1_position(position):
	pulse = camera_servo1_min_pulse + float(camera_servo1_max_pulse - camera_servo1_min_pulse) * (position - camera_servo1_min_angle) / (camera_servo1_max_angle - camera_servo1_min_angle)
	pulse = (pulse//10)*10
	camera_servo1.set_servo(camera_servo1_pin, pulse)
	
def set_camera_servo2_position(position):
	pulse = camera_servo2_min_pulse + float(camera_servo2_max_pulse - camera_servo2_min_pulse) * (position - camera_servo2_min_angle) / (camera_servo2_max_angle - camera_servo2_min_angle)
	pulse = (pulse//10)*10
	camera_servo2.set_servo(camera_servo2_pin, pulse)

def cleanup():
	PWM.cleanup()
	GPIO.cleanup()

	for thread in open_threads:
		thread.stop()
		thread.join()

class StoppableThread(threading.Thread):
	def __init__(self, *args, **kwargs):
		super(StoppableThread, self).__init__(*args, **kwargs)
		self._stop = threading.Event()

	def stop(self):
		self._stop.set()

	def stopped(self):
		return self._stop.isSet()

def poll(ws):
	while True:
		if ws._closed or threading.current_thread().stopped():
			break
		ws.write_message(u'send_input')
		time.sleep(polling_time)
	open_threads.remove(threading.current_thread())

open_threads = []

class KeyPressHandler(tornado.websocket.WebSocketHandler):

	throttle = 1.0

	def open(self):
		self._closed = False
		print 'opened socket'
		new_thread = StoppableThread(target = poll, args = (self,))
		open_threads.append(new_thread)
		new_thread.start()

	def on_message(self, message):
		msg = json.loads(message)
		# Arrow keys are sent in a binary format:
		# 1 - Up, 2 - Down, 4 - Left, 8 - Right
		if (msg.has_key('Keys')):
			arrows = msg['Keys']
			dir = [arrows & 1, (arrows & 2) >> 1, (arrows & 4) >> 2, (arrows & 8) >> 3]
			wheels = [-dir[0]+dir[1]+dir[2]-dir[3],dir[0]-dir[1]+dir[2]-dir[3]]
			# The left and right sets of wheels will always move in the same direction.
			# If a specific wheel needs to be addressed instead, use mfl or mbl
			w0 = '%.7f'% (wheels[0]*self.throttle)
			w1 = '%.7f'% (wheels[1]*self.throttle)
			mal = 'mal'+w0[:7]+':'
			mar = 'mar'+w1[:7]+':'
			print mal
			arduino_serial.write(mal)
			arduino_serial.write(mar+'\n')
		if (msg.has_key('Thr')):
			self.throttle = msg['Thr'] / 256.0

	def check_origin(self, origin):
		return True

	def on_close(self):
		self._closed = True

primary_phone_socket = None
class PhoneSocketHandler(tornado.websocket.WebSocketHandler):
	def open(self):
		self._closed = False
		print 'opened socket'
		new_thread = StoppableThread(target = poll, args = (self,))
		open_threads.append(new_thread)
		new_thread.start()

		self.zero_heading = -1

		self.target_heading_array = deque()
		self.target_pitch_array = deque()

		global primary_phone_socket
		if not primary_phone_socket:
			primary_phone_socket = self

	def on_message(self, message):
		global primary_phone_socket
		if self != primary_phone_socket:
			return

		# print 'Orientation:', message
		orientation = json.loads(message)
		if not (orientation.has_key('absolute') and orientation.has_key('alpha') and orientation.has_key('beta') and orientation.has_key('gamma')):
			return

		if orientation['gamma'] > 0:
			orientation['alpha'] = (orientation['alpha'] + 180)%360

		if self.zero_heading == -1:
			self.zero_heading = orientation['alpha']

		new_target_heading = orientation['alpha'] - self.zero_heading

		# change the heading range to [-180, 180]
		if new_target_heading > 180:
			new_target_heading -= 360
		if new_target_heading < -180:
			new_target_heading += 360

		new_target_heading = new_target_heading if new_target_heading >= camera_servo1_min_angle else camera_servo1_min_angle
		new_target_heading = new_target_heading if new_target_heading <= camera_servo1_max_angle else camera_servo1_max_angle

		self.target_heading_array.append(new_target_heading)
		while len(self.target_heading_array) > orientation_samples:
			self.target_heading_array.popleft()

		set_camera_servo1_position(sum(self.target_heading_array) / len(self.target_heading_array))

		if orientation['gamma'] > 0:
			new_target_pitch = 90 - orientation['gamma']
		else:
			new_target_pitch = -90 - orientation['gamma']
		new_target_pitch = new_target_pitch if new_target_pitch >= camera_servo2_min_angle else camera_servo2_min_angle
		new_target_pitch = new_target_pitch if new_target_pitch <= camera_servo2_max_angle else camera_servo2_max_angle

		self.target_pitch_array.append(new_target_pitch)
		while len(self.target_pitch_array) > orientation_samples:
			self.target_pitch_array.popleft()

		set_camera_servo2_position(sum(self.target_pitch_array) / len(self.target_pitch_array))

	def on_close(self):
		self._closed = True

		global primary_phone_socket
		primary_phone_socket = None

if __name__ == '__main__':
	application = tornado.web.Application([
		(r'/keysocket', KeyPressHandler),
		(r'/phonesocket', PhoneSocketHandler),
		(r'/(.*)', tornado.web.StaticFileHandler, { 'path': './www', 'default_filename': 'index.html' })
	])

	orientation_samples = 5
	polling_time = 0.1

	# m_fl_pwm_pin = 18
	# m_fl_gpio_pin = 17
	# m_fr_pwm_pin = 24
	# m_fr_gpio_pin = 23
	# m_bl_pwm_pin = 18
	# m_bl_gpio_pin = 17
	# m_br_pwm_pin = 24
	# m_br_gpio_pin = 23

	# motor_servo_fl = PWM.Servo(8)
	# motor_servo_fr = PWM.Servo(9)
	# motor_servo_bl = PWM.Servo(10)
	# motor_servo_br = PWM.Servo(11)
	# motor_servo_fl.set_servo(m_fl_pwm_pin, 0)
	# motor_servo_fr.set_servo(m_fr_pwm_pin, 0)
	# motor_servo_bl.set_servo(m_bl_pwm_pin, 0)
	# motor_servo_br.set_servo(m_br_pwm_pin, 0)

	# GPIO.setup(m_fl_gpio_pin, GPIO.OUT, initial = GPIO.LOW)
	# GPIO.setup(m_fr_gpio_pin, GPIO.OUT, initial = GPIO.LOW)
	# GPIO.setup(m_bl_gpio_pin, GPIO.OUT, initial = GPIO.LOW)
	# GPIO.setup(m_br_gpio_pin, GPIO.OUT, initial = GPIO.LOW)

	arduino_serial = serial.Serial('/dev/ttyUSB0', 115200);

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

	camera_servo1 = PWM.Servo(12)
	camera_servo2 = PWM.Servo(13)
	set_camera_servo1_position(0)
	set_camera_servo2_position(0)

	application.listen(80)
	try:
		tornado.ioloop.IOLoop.instance().start()
	except KeyboardInterrupt:
		cleanup()
	finally:
		cleanup()
