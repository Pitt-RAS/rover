import arduino_com
import time

arduino_com.init()

while(1):
    ping_sensor_distances = {'fl': -1, 'fr': -1, 'l': -1, 'r': -1, 'bl': -1, 'br': -1, 'b': -1}
    arduino_com.read_ping_sensors(ping_sensor_distances)
    print(ping_sensor_distances)
    time.sleep(1)