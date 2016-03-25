import serial
import struct

#-----------------------------------------------------
# set_camera_servo_position
# Sets the camera servo to a specified angle
#-----------------------------------------------------

def v_servo_write(position):
    #print("updating servo1: " + str(position))
    arduino_serial.write('sv' + 'v') # extra v is dummy character
    #The offset is because 120 degrees is actually 0 its constrained on the arduino
    for b in struct.pack('f', position):
        arduino_serial.write(b)
    arduino_serial.write(':')
    

def h_servo_write(position):
    #print("updating servo2: " + str(position))
    arduino_serial.write('sh' + 'h') #extra h is dummy character
    #The offset is because 150 degrees is actually 0 its constrained on the arduino
    for b in struct.pack('f', position):
        arduino_serial.write(b)
    arduino_serial.write(':')
    
    
def controlMotors(wheels, throttle):
    arduino_serial.write('mal')
    for b in struct.pack('f', wheels[0] * throttle):
        arduino_serial.write(b)
    arduino_serial.write(':')
    arduino_serial.write('mar')
    for b in struct.pack('f', wheels[1] * throttle):
        arduino_serial.write(b)
    arduino_serial.write(':')
    
def read_battery():
    arduino_serial.write('RA0')
    for b in struct.pack('f', 0):
        arduino_serial.write(b)
    arduino_serial.write(':')
    message = arduino_serial.readline()
    #print(message)
    voltage = ((int(message)/1024.0) * 5.13)
    battery_voltage = voltage/0.3625
    return battery_voltage


arduino_serial = serial.Serial('/dev/ttyACM0', 115200);
