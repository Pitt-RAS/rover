import serial
from serial import SerialException
import struct
import threading

serialLock = threading.Lock()
#-----------------------------------------------------
# set_camera_servo_position
# Sets the camera servo to a specified angle
#-----------------------------------------------------

def v_servo_write(position):
    write_data('s', arguments='vv', number=position)

def h_servo_write(position):
    write_data('s', arguments='hh', number=position)
    
def controlMotors(wheels, throttle):
    write_data('m', arguments='al', number=wheels[0] * throttle)
    write_data('m', arguments='ar', number=wheels[1] * throttle)

#this method needs to be refactored like the ones above but needs the communication protocol to change so
#that the serial lock is not lost when reading in a return value    
def read_battery():
    try:
        serialLock.acquire()
        arduino_serial.write('RA0')
        for b in struct.pack('f', 0):
            arduino_serial.write(b)
        arduino_serial.write(':')
        message = arduino_serial.readline()
        serialLock.release()
        voltage = ((int(message)/1024.0) * 5.13)
        battery_voltage = voltage/0.3625
        return battery_voltage
    except SerialException as e:
        print "SerialException error({0}): {1}".format(e.errno, e.strerror)
        return 99999

#this method needs to be refactored like the ones above but needs the communication protocol to change so
#that the serial lock is not lost when reading in a return value  
#------------------------------------------------------
# read_ping_sensors
# reads ping sensor distances (in cm) for the sensors named in the given dictionary
def read_ping_sensors(result):
    try:
        serialLock.acquire()
        for sensor in result:
            arduino_serial.write('p')
            arduino_serial.write(sensor.ljust(2))
            for b in struct.pack('f', 0):
                arduino_serial.write(b)
            arduino_serial.write(':')
            result[sensor] = float(arduino_serial.readline())
        serialLock.release()
    except SerialException as e:
        print "SerialException error({0}): {1}".format(e.errno, e.strerror)

def write_data(command, arguments='zz', number=0):
    try:
        serialLock.acquire()
        arduino_serial.write(command + arguments)
        for b in struct.pack('f', number):
            arduino_serial.write(b)
        arduino_serial.write(':')
        serialLock.release()
    except SerialException as e:
        print "SerialException error({0}): {1}".format(e.errno, e.strerror)

arduino_serial = serial.Serial('/dev/ttyACM0', 115200);
