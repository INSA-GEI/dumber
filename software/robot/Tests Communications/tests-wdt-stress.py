#!/usr/bin/python3

from time import sleep
import serial

def sendcmd(ser, cmd):
    print("\tcmd: " + str(cmd))
    ser.write(cmd)      # send command
    
    ans =ser.read(3)    # get answer
    print("\tans: " + str(ans))

def get_version(ser):
    cmd = b'VV\r'
    print("\tcmd: " + str(cmd))
    ser.write(cmd)      # send command
    
    ans =ser.read(5)    # get answer
    print("\tans: " + str(ans))

def main():
    ser = serial.Serial('/dev/ttyUSB0')  # open serial port on PC
    #ser = serial.Serial('/dev/ttyS0')  # open serial port on raspberry
    
    print("Open serial port " + ser.name) 

    print ("Ping robot\r")
    cmd = b'pp\r'
    sendcmd(ser, cmd)

    print ("Get version\r")
    get_version(ser)

    print ("Reset robot\r")
    cmd = b'rr\r'
    sendcmd(ser, cmd)

    print ("Start with watchdog\r")
    cmd = b'WW\r'
    sendcmd(ser, cmd)

    print ("######################################")
    print ("###       Watchdog started         ###")
    print ("######################################")

    #print ("Move -32767\r")
    #cmd = b'M=-32767j\r'
    #sendcmd(ser, cmd)

    while True:
        sleep (1.0)    # Attente de 1 seconde avant refresh du watchdog

        # On enchaine deux commandes pour voir comment le robot reagit
        print ("Move 20cms\r")
        cmd = b'M=20r\r'
        sendcmd(ser, cmd)

        print ("Reset watchdog\r")
        cmd = b'ww\r'
        sendcmd(ser, cmd)

try: 
    main()

except KeyboardInterrupt: # exception when pressing CTRL-C
    print ("Bye bye")