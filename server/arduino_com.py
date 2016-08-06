import serial
from serial import SerialException
import struct
import array
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
    send_command(5, data='v', number=position)

def h_servo_write(position):
    send_command(5, data='h', number=position)
    
def controlMotors(forwardV, rotationalV):
    #velocities range from -100 to 100 and we can't send negative numbers
    r = send_command(0, data=[100+forwardV, 100+rotationalV])

#this method needs to be refactored like the ones above but needs the communication protocol to change so
#that the serial lock is not lost when reading in a return value    
def read_battery():
    # Pin 59 corresponds to A5 on the arduino mega.
    adc_count = send_command(3, data=[59], results=2, dType = 'h')
    voltage = (adc_count/1024.0) * 5.02
    battery_voltage = voltage * 3.2243
    return battery_voltage

#this method needs to be refactored like the ones above but needs the communication protocol to change so
#that the serial lock is not lost when reading in a return value  
#------------------------------------------------------
# read_ping_sensors
# reads ping sensor distances (in cm) for the sensors named in the given dictionary
def read_ping_sensors(result):
    for sensor in result:
        result[sensor] = send_command(6, data=sensor.ljust(2), results=4, dType = 'f')

        
def led_rainbow(period):
  send_command(7, data=['r'], number=period)
  
def led_solid():
  send_command(7, data=['s'])
  
def led_off():
  send_command(7, data=['o'])
  
def led_color(r, g, b):
  send_command(7, data=['C', r, g, b])
  
def led_chasers(period):
  send_command(7, data = ['c'], number=period)

def led_rainbowfy(period):
  send_command(7, data=['R'], number=period)

def pack_bytes(list):
  string = ""
  for c in list:
    string = string + chr(c)
  return string

def send_command(command, data=[], results=0, number = float('nan'), dType = 'c'):
    try:
        serialLock.acquire()
        
        #tack float bytes on for legacy support
        if(number != float('nan')):
          if isinstance(data, str):
            for b in struct.pack('f', number):
              data = data + b
          if isinstance(data, list):
            for b in struct.pack('f', number):
              data.append(b)
            data = str(bytearray(data))
        
        handshake()
        write_data(command, data)
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

def write_data(command, data):
  success = "no"
  while(success != "ok"):
    final_data = chr(command) + data
    arduino_serial.write(chr(len(final_data)))
    arduino_serial.write(final_data)
    success = arduino_serial.readline().rstrip()

def handshake():
  success = "no"
  while(success != "go"):
    arduino_serial.write(':')
    success = arduino_serial.readline().rstrip()

arduino_serial = serial.Serial('/dev/ttyACM0', 115200, timeout=0.5)
