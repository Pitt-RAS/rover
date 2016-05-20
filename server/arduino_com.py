import serial
from serial import SerialException
import struct
import threading
from time import sleep

serialLock = threading.Lock()
numArguments = 4

def init():
  sleep(2) #make the arduino had time to come online
  arduino_serial.flushInput()
  arduino_serial.flushOutput()

#-----------------------------------------------------
# set_camera_servo_position
# Sets the camera servo to a specified angle
#-----------------------------------------------------
def v_servo_write(position):
    send_command('s', arguments='vv', number=position)

def h_servo_write(position):
    send_command('s', arguments='hh', number=position)
    
def controlMotors(wheels, throttle):
    send_command('m', arguments='al', number=wheels[0] * throttle)
    send_command('m', arguments='ar', number=wheels[1] * throttle)

#this method needs to be refactored like the ones above but needs the communication protocol to change so
#that the serial lock is not lost when reading in a return value    
def read_battery():
    adc_count = send_command('R', arguments="A0", results=2, dType = 'h')
    voltage = (adc_count/1024.0) * 5.13
    battery_voltage = voltage/0.3625
    return battery_voltage

#this method needs to be refactored like the ones above but needs the communication protocol to change so
#that the serial lock is not lost when reading in a return value  
#------------------------------------------------------
# read_ping_sensors
# reads ping sensor distances (in cm) for the sensors named in the given dictionary
def read_ping_sensors(result):
    for sensor in result:
        result[sensor] = send_command('p', arguments=sensor.ljust(2), results=4, dType = 'f')

        
def led_rainbow(period):
  send_command('l', arguments="r", number=period)
  
def led_solid(r, g, b):
  color = pack_bytes([r, g, b])
  send_command('l', arguments='s' + color)
  
def led_off():
  send_command('l', arguments="o")
  
def led_chasers(r, g, b, period):
  color = pack_bytes([r, g, b])
  send_command('l', arguments = 'c' + color, number=period)

def pack_bytes(list):
  string = ""
  for c in list:
    string = string + chr(c)
  return string

def send_command(command, arguments="", number=0, results=0, dType = 'c'):
    try:
        serialLock.acquire()
        
        if isinstance(arguments, list):
          arguments = " ".join(chr, arguments)
                  
        arguments = arguments.ljust(numArguments)
        handshake()
        write_data(command, arguments, number)
        return_data = read_data(results, dType)
        serialLock.release()  
        return return_data

    except SerialException as e:
        print "SerialException error({0}): {1}".format(e.errno, e.strerror)
        return

def read_data(results, dType):
  if(results > 0):
    data = arduino_serial.read(results)
    return struct.unpack(dType, data)[0]
  return None


def write_data(command, arguments, number):

  success = "no"
  while(success != "ok"):
    arduino_serial.write(command + arguments)
    for b in struct.pack('f', number):
      arduino_serial.write(b)
    arduino_serial.write(';')
    success = arduino_serial.readline().rstrip()

def handshake():
  success = "no"
  while(success != "go"):
    arduino_serial.write(':')
    success = arduino_serial.readline().rstrip()

arduino_serial = serial.Serial('/dev/ttyACM0', 115200, timeout=0.5)
