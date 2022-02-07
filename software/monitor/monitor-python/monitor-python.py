#!/usr/bin/env python3

import curses
from curses.ascii import ACK
from email.message import Message
import os
import socket
import threading
from  queue import Queue
import time
import sys

class GlobVar:
    port = 5544              # default server port
    timeout=1.0              # default waiting time for an answer. May be increased for debugging purpose
    address = "localhost"    # default server address. Use this with 'nc -l <port>' for testing on your machine
    getBatteryLevelPeriod=-1 # periodic delay for requesting battery level. If value is 0 or below, not battery check is done

    # do not modify value below this line
    
    connectedToPi = False
    connectedToDumber = False
    dumberStarted = False
    
    batteryLevel = -1
    currentMovement="STOP"
    
    last_answer = -1
    
    message = []
    
    exceptionmsg= ""
    
# Network event from socket. 
# Event are 'Connected to server' or 'disconnected from server'
class NetworkEvents:
    EVENT_CONNECTED=1
    EVENT_CONNECTION_LOST=0
    UNKNOWN_EVENT=-1
    
# Network managment class
# Contain a thread for conenction to server and data reception
# Contains functions for sending message to server
# Contains functions for answer decoding
class Network():
    receiveCallback=None
    eventCallback=None
    reconnectionDelay =0
    waitForAnswer=False
    receiveEvent = None
    receivedAnswer = ""
    
    # List of possible answers from server
    ANSWER_ACK = "AACK"
    ANSWER_NACK = "ANAK"
    ANSWER_COM_ERROR = "ACER"
    ANSWER_TIMEOUT = "ATIM"
    ANSWER_CMD_REJECTED = "ACRJ"
    
    # List of possible messages from server
    MESSAGE = "MSSG"
    ROBOT_BATTERY_LEVEL = "RBLV"
    ROBOT_CURRENT_STATE = "RCST"
    
    # List of accepted command by server
    CAMERA_OPEN = "COPN"
    CAMERA_CLOSE = "CCLS"
    CAMERA_IMAGE = "CIMG"
    CAMERA_ARENA_ASK = "CASA"
    CAMERA_ARENA_INFIRM = "CAIN"
    CAMERA_ARENA_CONFIRM = "CACO"
    CAMERA_POSITION_COMPUTE = "CPCO"
    CAMERA_POSITION_STOP = "CPST"
    CAMERA_POSITION = "CPOS"
    ROBOT_COM_OPEN = "ROPN"
    ROBOT_COM_CLOSE = "RCLS"
    ROBOT_PING = "RPIN"
    ROBOT_RESET = "RRST"
    ROBOT_START_WITHOUT_WD = "RSOW"
    ROBOT_START_WITH_WD = "RSWW"
    ROBOT_RELOAD_WD = "RLDW"
    ROBOT_MOVE = "RMOV"
    ROBOT_TURN = "RTRN"
    ROBOT_GO_FORWARD = "RGFW"
    ROBOT_GO_BACKWARD = "RGBW"
    ROBOT_GO_LEFT = "RGLF"
    ROBOT_GO_RIGHT = "RGRI"
    ROBOT_STOP = "RSTP"
    ROBOT_POWEROFF = "RPOF"
    ROBOT_GET_BATTERY = "RGBT"
    ROBOT_GET_STATE = "RGST"

    SEPARATOR_CHAR = ':'
    
    # Decoded answers
    ACK = 1 
    NACK =2
    COM_ERROR =3
    TIMEOUT_ERROR=4
    CMD_REJECTED=5
    UNKNOWN=-1
    
    # Initialisation method.
    def __init__(self, receiveCallback=None, eventCallback=None) -> None:
        self.receiveCallback = receiveCallback
        self.eventCallback = eventCallback
        self.waitForAnswer=False
        self.receiveEvent = threading.Event()
    
    # Reception thread used for conencting to server and receiving data.
    # Never stop, auto reconnect if connection is lost
    # Send event when conencting to or disconnecting from server
    # send received message to appropriate callback
    def ReceptionThread(self) -> None:   
        while True:
            try:
                self.__connect()
                
                if self.eventCallback != None:
                    self.eventCallback(NetworkEvents.EVENT_CONNECTED)
                self.__read()
                
            except (RuntimeError, TimeoutError, socket.gaierror, ConnectionResetError) as e:
                if e.__class__ == RuntimeError or e.__class__== ConnectionResetError:
                    if self.eventCallback != None:
                        self.eventCallback(NetworkEvents.EVENT_CONNECTION_LOST)
            
            except Exception as e:
                pass
            
            time.sleep(2.0)
         
    # Method helper for starting reception thread    
    def startReceptionThread(self) -> None:
        self.threadId = threading.Thread(target=self.ReceptionThread, args=())
        self.threadId.daemon=True
        self.threadId.start()
            
    # Private method for connecting to server     
    def __connect(self) -> None:
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        try:
            self.sock.connect((GlobVar.address, GlobVar.port))
        except ConnectionRefusedError: 
            raise RuntimeError("Unable to connect to " + GlobVar.address + ":" + str(GlobVar.port))
    
        self.waitForAnswer=False
        
    # Private method for sending raw data to server
    def __write(self, msg: str) -> None:
        totalsent = 0
        if msg[-1] != '\n':
            msg = msg + '\n'
            
        MSGLEN = len(msg)
        msgbytes = bytes(msg,'UTF-8')
        
        while totalsent < MSGLEN:
            sent = self.sock.send(msgbytes[totalsent:])
            if sent == 0:
                GlobVar.connectedToPi=False
                raise RuntimeError("Unable to send data to " + GlobVar.address + ":" + str(GlobVar.port))
            totalsent = totalsent + sent
            
    # Private method for receiving data from server. 
    # Data received are sent to callback __receiveHandler for decoding
    def __read(self) -> None:
        chunks = []
        bytes_recd = 0
        last_char=0
        
        while True:
            while last_char != 0x0A:
                chunk = self.sock.recv(2048)
                if chunk == b'':
                    raise RuntimeError("Connection lost with " + GlobVar.address + ":" + str(GlobVar.port))
                chunks.append(chunk)
                bytes_recd = bytes_recd + len(chunk)
                last_char=chunk[-1]
            
            self.__receiveHandler(b''.join(chunks).decode("utf-8"))
            chunks = []
            bytes_recd =0
            last_char=0
    
    # private callback for reception and decoding of data. 
    # If no answer is wait, send data to caller callback
    def __receiveHandler(self, data: str) -> None:
        # traitement a faire lors de la reception de donnée
        if self.waitForAnswer:
            self.receivedAnswer = data
            self.waitForAnswer=False
            self.receiveEvent.set()
        else:
            # si pas de donnée prevue en reception, alors envoie au callback
            if self.receiveCallback != None:
                self.receiveCallback(data)

    # Private method for sending command to server
    def __sendCommand(self,cmd, waitans: bool) -> str:
        if waitans == True:
            self.waitForAnswer = True
        else:
            self.waitForAnswer = False
            
        self.__write(cmd)
        
        if waitans:
            ans = self.ANSWER_TIMEOUT
            try:
                self.receiveEvent.wait(timeout=GlobVar.timeout) # Max waiting time = GlobVar.timeout
                
                if self.receiveEvent.is_set():
                    ans=self.receivedAnswer
                    self.receivedAnswer=""
                else:
                    ans=self.ANSWER_TIMEOUT
                       
                self.receiveEvent.clear()
            except TimeoutError:
                ans=self.ANSWER_TIMEOUT
            except Exception as e:
                GlobVar.exceptionmsg = str(e)
                
            self.waitForAnswer=False
            return ans
        else:
            return self.ANSWER_ACK
               
    # Private method for decoding answer from server     
    def __decodeAnswer(self, ans: str) -> int:
        if self.ANSWER_ACK in ans:
            return self.ACK
        elif self.ANSWER_NACK in ans:
            return self.NACK
        elif self.ANSWER_COM_ERROR in ans:
            return self.COM_ERROR
        elif self.ANSWER_TIMEOUT in ans:
            return self.TIMEOUT_ERROR
        elif self.ANSWER_CMD_REJECTED in ans:
            return self.CMD_REJECTED
        else:
            return self.UNKNOWN
    
    # Send OpenCom command to server
    def robotOpenCom(self) -> int:
        ans = self.__sendCommand(self.ROBOT_COM_OPEN, True)
        return self.__decodeAnswer(ans)
    
    # Send CloseCom command to server
    def robotCloseCom(self) -> int:
        ans = self.__sendCommand(self.ROBOT_COM_CLOSE, True)
        return self.__decodeAnswer(ans)
    
    # Send StartWithWatchdog command to server
    def robotStartWithWatchdog(self) -> int:
        ans = self.__sendCommand(self.ROBOT_START_WITH_WD, True)
        return self.__decodeAnswer(ans)
    
    # Send StartWithoutWatchdog command to server
    def robotStartWithoutWatchdog(self) -> int:
        ans = self.__sendCommand(self.ROBOT_START_WITHOUT_WD, True)
        return self.__decodeAnswer(ans)
    
    # Send Reset command to server
    def robotReset(self) -> int:
        ans = self.__sendCommand(self.ROBOT_RESET, True)
        return self.__decodeAnswer(ans)
    
    # Send Stop command to server
    def robotStop(self) -> int:
        ans = self.__sendCommand(self.ROBOT_STOP, True)
        return self.__decodeAnswer(ans)
    
    # Send GoLeft command to server
    def robotGoLeft(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_LEFT, True)
        return self.__decodeAnswer(ans)
    
    # Send GoRight command to server
    def robotGoRight(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_RIGHT, True)
        return self.__decodeAnswer(ans)
    
    # Send GoForward command to server
    def robotGoForward(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_FORWARD, True)
        return self.__decodeAnswer(ans)
    
    # Send GoBackward command to server
    def robotGoBackward(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_BACKWARD, True)
        return self.__decodeAnswer(ans)
    
    # Send GetBattery command to server
    def robotGetBattery(self) -> None:
        ans = self.__sendCommand(self.ROBOT_GET_BATTERY,False)
        
# Function for decoding battery level
def batterylevelToStr(batlvl: int) -> str:
    switcher = {
        2: "Full",
        1: "Mid",
        0: "Empty",
    }
    
    return switcher.get(batlvl, "Unknown")

# Function for display human readable answer
def answertoStr(ans: int) -> str:
    switcher = {
        Network.ACK: "Acknowledged",
        Network.NACK: "Not acknowledged",
        Network.TIMEOUT_ERROR: "No answer (timeout)",
        Network.CMD_REJECTED: "Command rejected",
        Network.COM_ERROR: "Communication error (invalid command)"
    }
    
    return switcher.get(ans, "Unknown answer")

# Thread for updtating display
def threadRefreshScreen(currentWindow) -> None:
    while 1:
        currentWindow.clear()
        currentWindow.addstr("Connected to " + GlobVar.address + ":" + str(GlobVar.port) +" = " + str(GlobVar.connectedToPi))
        currentWindow.move(1,0)
        currentWindow.addstr("Connected to robot = " + str(GlobVar.connectedToDumber))
        currentWindow.move(3,0)
        currentWindow.addstr("Robot started = " + str(GlobVar.dumberStarted))
        currentWindow.move(4,0)
        currentWindow.addstr("Current movement sent to robot = " + str(GlobVar.currentMovement))
        currentWindow.move(5,0)
        currentWindow.addstr("Battery level = " + batterylevelToStr(GlobVar.batteryLevel))
        currentWindow.move(7,0)
        currentWindow.addstr("Last answer received = " + answertoStr(GlobVar.last_answer))
        currentWindow.move(9,0)
        currentWindow.addstr("Messages received (log)")
        
        # up to 8 messages
        for i in range(0,len(GlobVar.message)):
            currentWindow.move(10+i,0)
            currentWindow.addstr("[mes "+str(i)+ "] ")
            currentWindow.addstr(str(GlobVar.message[i]))
        
        currentWindow.move(20,0)
        currentWindow.addstr("Commands : \'O\' = Open COM with robot/ \'C\' = Close COM with robot")
        currentWindow.move(21,0)
        currentWindow.addstr("           \'W\' = Start robot with wdt/ \'U\' = Start robot without wdt/ \'R\' = Reset robot")
        currentWindow.move(22,0)
        currentWindow.addstr("           \'Up\',\'Down\',\'Left\',\'Right\' = Move robot/ Space Bar = Stop robot")
        currentWindow.move(23,0)
        currentWindow.addstr("           CTRL-C = Quit program")
        currentWindow.refresh()
        time.sleep(0.5)

# Thread for reading keyboard keys and sending corresponding commands to server
def threadGetKeys(win: curses.window, net:Network) -> None:
    while 1: 
        try:                          
            key = win.getkey() 
            win.addstr(" KEY = " + key)
            
            GlobVar.currentMovement = "Stop"
            if GlobVar.connectedToPi:
                if key == "KEY_UP":
                    GlobVar.currentMovement = "Go Forward"
                    GlobVar.last_answer = net.robotGoForward()
                elif key == "KEY_DOWN":
                    GlobVar.currentMovement = "Go Backward"
                    GlobVar.last_answer = net.robotGoBackward()
                elif key == "KEY_RIGHT":
                    GlobVar.currentMovement = "Go Right"
                    GlobVar.last_answer = net.robotGoRight()
                elif key == "KEY_LEFT":
                    GlobVar.currentMovement = "Go Left"
                    GlobVar.last_answer = net.robotGoLeft()
                elif key == " ":
                    GlobVar.currentMovement = "Stop" 
                    GlobVar.last_answer = net.robotStop() 
                elif key == "R" or key == 'r':
                    GlobVar.last_answer = net.robotReset() 
                    GlobVar.dumberStarted=False
                elif key == "U" or key == 'u':
                    GlobVar.last_answer = net.robotStartWithoutWatchdog()
                    if GlobVar.last_answer == net.ACK:
                        GlobVar.dumberStarted=True
                    else:
                        GlobVar.dumberStarted=False 
                elif key == "W" or key == 'w':
                    GlobVar.last_answer = net.robotStartWithWatchdog() 
                    if GlobVar.last_answer == net.ACK:
                        GlobVar.dumberStarted=True
                    else:
                        GlobVar.dumberStarted=False 
                elif key == "C" or key=='c':
                    GlobVar.last_answer = net.robotCloseCom()
                    GlobVar.connectedToDumber=False
                    GlobVar.dumberStarted=False
                elif key == "O" or key =='o':
                    GlobVar.last_answer = net.robotOpenCom() 
                    if GlobVar.last_answer == net.ACK:
                        GlobVar.connectedToDumber=True
                    else:
                        GlobVar.connectedToDumber=False         
            
            #if key == os.linesep or key =='q' or key == 'Q':
            #    break  
            
        except Exception as e:
            GlobVar.exceptionmsg="Exception received: " + str(e)

# Thread used for requesting battery level, exit if getBatteryLevelPeriod is 0 or below
def threadPeriodic(net: Network) -> None:
    while True:
        if GlobVar.getBatteryLevelPeriod>0:
            time.sleep(GlobVar.getBatteryLevelPeriod)
        
            GlobVar.batteryLevel = -1
            if GlobVar.connectedToPi:
                net.robotGetBattery() 
        else:
            break
        
# Callback used to decode non answer message from server (mainly battery level and log message)
def receptionCallback(s:str) -> None:
    if Network.ROBOT_BATTERY_LEVEL in s:
        str_split = s.split(':')
        
        try:
            GlobVar.batteryLevel = int(str_split[1])
        except:
            GlobVar.batteryLevel = -1
    
    if Network.MESSAGE in s:
        str_split = s.split(':')
        
        try:
            GlobVar.message.append(str(str_split[1]))
        except Exception as e:
            GlobVar.exceptionmsg = str(e)
        
        if len(GlobVar.message) > 8:
            GlobVar.message.pop(0)

# Callback for connection/deconnection event from network manager
def eventCallback(event: NetworkEvents) -> None:
    if event == NetworkEvents.EVENT_CONNECTED:
        GlobVar.connectedToPi = True
    elif event == NetworkEvents.EVENT_CONNECTION_LOST:
        GlobVar.connectedToPi = False

# Main program, wait for keys thread to end (CTRL-C)
def main(win: curses.window) -> None:
    win.keypad(True)
    
    net = Network(receiveCallback=receptionCallback, eventCallback=eventCallback)
    
    net.startReceptionThread()
     
    windowThread = threading.Thread(target=threadRefreshScreen, args=(win,))
    windowThread.daemon=True
    windowThread.start()

    keyThread = threading.Thread(target=threadGetKeys, args=(win,net,))
    keyThread.daemon=True
    keyThread.start()

    periodicThread = threading.Thread(target=threadPeriodic, args=(net,))
    periodicThread.daemon=True
    periodicThread.start()

    keyThread.join()
    
    while True:
        time.sleep(2.0)
 
try: 
    if len(sys.argv)>=2:
        GlobVar.address = sys.argv[1]
    else:
        print ("No target address specified")
        print ("Usage: monitor-python.py address [port]")
        
        #exit (-1) # Comment this line for connecting to localhost
        
    if len(sys.argv)>=3:
        GlobVar.port = int(sys.argv[2])
        
    curses.wrapper(main) 
    
except KeyboardInterrupt: # exception when pressing CTRL-C
    print ("Bye bye")

