import arduino_com
import time

arduino_com.init();

while(1):
  '''print("off")
  arduino_com.led_off();
  time.sleep(1)
  
  print("solid")
  for x in range(0, 255 , 10  ):
    arduino_com.led_solid(x, 0, 255 - x)
    time.sleep(0.1)
    print(x)
  
  print("rainbow")
  for x in range(0, 20):
    arduino_com.led_rainbow(25 - x)
    time.sleep(0.3)
  time.sleep(5)'''
  
  print("chasers")
  arduino_com.led_chasers(0, 128, 0, -5)
  time.sleep(5)