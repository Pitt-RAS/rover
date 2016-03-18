import os
import time

def get_wifi_strength():
    f = open('/proc/net/wireless', 'r')
    f.readline()
    f.readline()
    line = f.readline()
    parsed = line.split()
    return parsed[2]


    
#-----------------------------------------------------
# Program starts here
#-----------------------------------------------------
if __name__ == '__main__':
    while(1):
        get_wifi_strength()
        time.sleep(1)
