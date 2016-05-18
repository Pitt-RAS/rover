import arduino_com
import time

arduino_com.init();

while(1):
    print(str(arduino_com.read_battery()) + 'V')
    time.sleep(1)