#!/usr/bin/python3

from time import sleep
import serial

def sendcmd(ser, cmd):
    print("\tcmd: " + str(cmd))
    ser.write(cmd)      # send command
    
    ans =ser.read(3)    # get answer
    print("\tans: " + str(ans))

def main():
    ser = serial.Serial('/dev/ttyUSB0')  # open serial port on PC
    #ser = serial.Serial('/dev/ttyS0')  # open serial port on raspberry
    print("Open serial port " + ser.name) 

    print ("Ping robot\r")
    cmd = b'pp\r'
    sendcmd(ser, cmd)

    print ("Reset robot\r")
    cmd = b'rr\r'
    sendcmd(ser, cmd)

    print ("Start without watchdog\r")
    cmd = b'uu\r'
    sendcmd(ser, cmd)

    print ("Move 20cms\r")
    cmd = b'M=20r\r'
    sendcmd(ser, cmd)

    sleep (4)

    print ("Reset robot\r")
    cmd = b'rr\r'
    sendcmd(ser, cmd)

    ser.close()
    print ("Bye bye")

try: 
    main()

except KeyboardInterrupt: # exception when pressing CTRL-C
    print ("Bye bye")