# -*- coding: utf-8 -*-

class GlobVar:
    port = 5544              # default server port
    timeout=1.0              # default waiting time for an answer. May be increased for debugging purpose
    address = "localhost"    # default server address. Use this with 'nc -l <port>' for testing on your machine
    getBatteryLevelPeriod=2  # periodic delay for requesting battery level. If value is 0 or below, not battery check is done

    # do not modify value below this line
    
    connectedToPi = False
    connectedToDumber = False
    dumberStarted = False
    
    batteryLevel = -1
    currentMovement="STOP"
    
    last_answer = -1
    
    #message = []
    
    exceptionmsg= ""