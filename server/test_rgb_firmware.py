import arduino_com_rgb
import time

time.sleep(2)

arduino_com_rgb.rainbow(50);

while(1):
    '''print("red")
    arduino_com_rgb.solid(65, 0, 0)
    time.sleep(1)
    print("green")
    arduino_com_rgb.solid(0, 54, 0)
    time.sleep(1)
    print("off")
    arduino_com_rgb.off()
    time.sleep(1)'''
