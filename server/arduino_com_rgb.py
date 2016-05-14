import serial
import struct
import threading

serialLock = threading.Lock()
#-----------------------------------------------------
# set_camera_servo_position
# Sets the camera servo to a specified angle
#-----------------------------------------------------

def off():
    serialLock.acquire()
    arduino_serial.write(':')
    arduino_serial.write('o' + 'ooo') # extra v is dummy character
    for b in struct.pack('f', 50.0):
        arduino_serial.write(b)
    arduino_serial.write(';')
    serialLock.release()

def solid(r, g, b):
    serialLock.acquire()
    arduino_serial.write(':')
    arduino_serial.write('s' + chr(r) + chr(g) + chr(b))
    for b in struct.pack('f', 50.0):
        arduino_serial.write(b)
    arduino_serial.write(';')
    serialLock.release()

arduino_serial = serial.Serial('/dev/ttyUSB0', 115200, timeout=1);
