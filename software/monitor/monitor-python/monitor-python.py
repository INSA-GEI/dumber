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
    connectedToPi = False
    port = 5544
    timeout=1000
    address = "10.105.1.13"

    connectedToDumber = False
    dumberStarted = False
    
    batteryLevel = -1
    currentMovement="STOP"
    
    last_answer = -1
    
    message = []
    
    exceptionmsg= ""
    
class NetworkEvents:
    EVENT_CONNECTED=1
    EVENT_CONNECTION_LOST=0
    UNKNOWN_EVENT=-1
    
class Network():
    receiveCallback=None
    eventCallback=None
    reconnectionDelay =0
    waitForAnswer=False
    receiveEvent = None
    receivedAnswer = ""
    
    ANSWER_ACK = "AACK"
    ANSWER_NACK = "ANAK"
    ANSWER_COM_ERROR = "ACER"
    ANSWER_TIMEOUT = "ATIM"
    ANSWER_CMD_REJECTED = "ACRJ"
    MESSAGE = "MSSG"
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
    ROBOT_BATTERY_LEVEL = "RBLV"
    ROBOT_GET_BATTERY = "RGBT"
    ROBOT_GET_STATE = "RGST"
    ROBOT_CURRENT_STATE = "RCST"

    SEPARATOR_CHAR = ':'
    
    ACK = 1 
    NACK =2
    COM_ERROR =3
    TIMEOUT_ERROR=4
    CMD_REJECTED=5
    UNKNOWN=-1
    
    def __init__(self, receiveCallback=None, eventCallback=None) -> None:
        self.receiveCallback = receiveCallback
        self.eventCallback = eventCallback
        self.waitForAnswer=False
        self.receiveEvent = threading.Event()
    
    def ReceptionThread(self):   
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
             
    def startReceptionThread(self) -> None:
        self.threadId = threading.Thread(target=self.ReceptionThread, args=())
        self.threadId.daemon=True
        self.threadId.start()
                 
    def __connect(self) -> None:
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        try:
            self.sock.connect((GlobVar.address, GlobVar.port))
        except ConnectionRefusedError: 
            raise RuntimeError("Unable to connect to " + GlobVar.address + ":" + str(GlobVar.port))
    
        self.waitForAnswer=False
        
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

    def __sendCommand(self,cmd, waitans) -> str:
        if waitans == True:
            self.waitForAnswer = True
        else:
            self.waitForAnswer = False
            
        self.__write(cmd)
        
        if waitans:
            ans = self.ANSWER_TIMEOUT
            try:
                self.receiveEvent.wait(timeout=1.0) # timeout set to 0.3 sec
                
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
                    
    def __decodeAnswer(self, ans) -> int:
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
    
    def robotOpenCom(self) -> int:
        ans = self.__sendCommand(self.ROBOT_COM_OPEN, True)
        return self.__decodeAnswer(ans)
    
    def robotCloseCom(self) -> int:
        ans = self.__sendCommand(self.ROBOT_COM_CLOSE, True)
        return self.__decodeAnswer(ans)
    
    def robotStartWithWatchdog(self) -> int:
        ans = self.__sendCommand(self.ROBOT_START_WITH_WD, True)
        return self.__decodeAnswer(ans)
    
    def robotStartWithoutWatchdog(self) -> int:
        ans = self.__sendCommand(self.ROBOT_START_WITHOUT_WD, True)
        return self.__decodeAnswer(ans)
    
    def robotReset(self) -> int:
        ans = self.__sendCommand(self.ROBOT_RESET, True)
        return self.__decodeAnswer(ans)
    
    def robotStop(self) -> int:
        ans = self.__sendCommand(self.ROBOT_STOP, True)
        return self.__decodeAnswer(ans)
    
    def robotGoLeft(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_LEFT, True)
        return self.__decodeAnswer(ans)
    
    def robotGoRight(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_RIGHT, True)
        return self.__decodeAnswer(ans)
    
    def robotGoForward(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_FORWARD, True)
        return self.__decodeAnswer(ans)
    
    def robotGoBackward(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_BACKWARD, True)
        return self.__decodeAnswer(ans)
    
    def robotGetBattery(self) -> None:
        ans = self.__sendCommand(self.ROBOT_GET_BATTERY,False)
        
def batterylevelToStr(batlvl: int) -> str:
    switcher = {
        2: "Full",
        1: "Mid",
        0: "Empty",
    }
    
    return switcher.get(batlvl, "Unknown")

def answertoStr(ans: int) -> str:
    switcher = {
        Network.ACK: "Acknowledged",
        Network.NACK: "Not acknowledged",
        Network.TIMEOUT_ERROR: "No answer (timeout)",
        Network.CMD_REJECTED: "Command rejected",
        Network.COM_ERROR: "Communication error (invalid command)"
    }
    
    return switcher.get(ans, "Unknown answer")

def threadRefreshScreen(currentWindow):
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
        for i in range(0,len(GlobVar.message)):
            currentWindow.move(10+i,0)
            currentWindow.addstr("[mes "+str(i)+ "] ")
            currentWindow.addstr(str(GlobVar.message[i]))
        
        # up to 8 messages
        
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

def threadGetKeys(win, net:Network):
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

def threadPeriodic(net: Network):
    while True:
        time.sleep(5.0)
        
        GlobVar.batteryLevel = -1
        if GlobVar.connectedToPi:
            net.robotGetBattery() 
        
def receptionCallback(s:str):
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

def eventCallback(event):
    if event == NetworkEvents.EVENT_CONNECTED:
        GlobVar.connectedToPi = True
    elif event == NetworkEvents.EVENT_CONNECTION_LOST:
        GlobVar.connectedToPi = False

def main(win):
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
    # periodicThread.start()

    keyThread.join()
    
    while True:
        time.sleep(2.0)
 
try: 
    if len(sys.argv)>=2:
        GlobVar.address = sys.argv[1]
    else:
        print ("No target address specified")
        print ("Usage: monitor-python.py address [port]")
        
        exit (-1) 
        #GlobVar.address = "10.105.1.12"
        
    if len(sys.argv)>=3:
        GlobVar.port = int(sys.argv[2])
    else:
        GlobVar.port = 5544
        
    curses.wrapper(main) 
    
except KeyboardInterrupt:
    print ("Bye bye")
except:
    raise

