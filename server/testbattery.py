import arduino_com
import time

while(1):
    print(str(arduino_com.read_battery()) + 'V')
    time.sleep(1)