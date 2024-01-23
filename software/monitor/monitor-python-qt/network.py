# -*- coding: utf-8 -*-

import socket
import time
import sys

from PyQt5 import QtCore
from globvar import GlobVar

import ipaddress
import threading

# Network event from socket. 
# Event are 'Connected to server' or 'disconnected from server'
class NetworkEvents():
    EVENT_CONNECTED=1
    EVENT_CONNECTION_LOST=0
    UNKNOWN_EVENT=-1

class NetworkAnswers():
     # Decoded answers
    ACK = 1 
    NACK =2
    COM_ERROR =3
    TIMEOUT_ERROR=4
    CMD_REJECTED=5
    UNKNOWN=-1
    
# Network managment class
# Contain a thread for conenction to server and data reception
# Contains functions for sending message to server
# Contains functions for answer decoding
class Network(QtCore.QThread):
    i=0
    reconnectionDelay =0
    waitForAnswer=False
    receivedAnswer = ""
    receiveFlag = None
    
    receptionEvent = QtCore.pyqtSignal(str)
    connectionEvent= QtCore.pyqtSignal(int)
    answerEvent= QtCore.pyqtSignal(int)
    logEvent = QtCore.pyqtSignal(str)
    
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
   
    def __init__(self):
        super(Network, self).__init__()
        self.i =0
        self.receiveFlag = threading.Event()
        
    def run(self):
        print ("Network thread started" )
        self.i =0
        
        while True:
            self.i = self.i+1
                       
            try:
                self.__connect()
                            
                self.connectionEvent.emit(NetworkEvents.EVENT_CONNECTED)
                self.__read()
                
            except (RuntimeError, TimeoutError, socket.gaierror, ConnectionResetError, ConnectionRefusedError) as e:
                print ("exception in network thread: " + str(e))
                if e.__class__ == RuntimeError or \
                   e.__class__== ConnectionResetError or \
                   e.__class__ == ConnectionRefusedError:
                    self.connectionEvent.emit(NetworkEvents.EVENT_CONNECTION_LOST)
                
            except Exception as e:
                print ("unknown exception in network thread: " + str(e))
                       
            time.sleep(2.0)
    
    # Private method for connecting to server     
    def __connect(self) -> None:
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        try:
            self.sock.connect((GlobVar.address, GlobVar.port))
        except ConnectionRefusedError: 
            raise RuntimeError("Unable to connect to " + GlobVar.address + ":" + str(GlobVar.port))
    
        # In UDP, no way to know if server is running (connect is always successfull). 
        # So, send a single line feed to ping the server
        #self.sock.send(str.encode("\n"))
        self.waitForAnswer=False    
        
    # Private method for receiving data from server. 
    # Data received are sent to callback __receiveHandler for decoding
    def __read(self) -> None:
        chunks = []
        bytes_recd = 0
        last_char=0
        
        while True:
            while last_char != 0x0A:
                chunk = self.sock.recv(2048)
                #print ("Chunk =" + str(chunk))
                
                if chunk == b'':
                    raise RuntimeError("Connection lost with " + GlobVar.address + ":" + str(GlobVar.port))
                chunks.append(chunk)
                bytes_recd = bytes_recd + len(chunk)
                last_char=chunk[-1]
            
            #print ("")
            #print ("Data: ")
            #print (b''.join(chunks))
            #print ("")
            
            self.__receiveHandler(b''.join(chunks).decode("utf-8"))
            chunks = []
            bytes_recd =0
            last_char=0
    
    # private callback for reception and decoding of data. 
    # If no answer is wait, send data to caller callback
    def __receiveHandler(self, data: str) -> None:
        # traitement a faire lors de la reception de donnée
        self.logEvent.emit("< " + data)
        
        if self.waitForAnswer:
            self.receivedAnswer = data
            self.waitForAnswer=False
            self.receiveFlag.set()
        else:
            # si pas de donnée prevue en reception, alors envoie au callback
            self.receptionEvent.emit(data)
    
    def checkAddressIsValid(self, address: str) -> bool:
        try:
            if address == "localhost":
                return True
            
            ip=ipaddress.ip_address(address)
            return True
        
        except ValueError as e:
            return False
     
    # Private method for sending raw data to server
    def __write(self, msg: str) -> None:
        totalsent = 0
        if msg[-1] != '\n':
            msg = msg + '\n'
            
        MSGLEN = len(msg)
        msgbytes = bytes(msg,'UTF-8')
        
        self.logEvent.emit ("> " + msg)
        
        while totalsent < MSGLEN:
            sent = self.sock.send(msgbytes[totalsent:])
            if sent == 0:
                GlobVar.connectedToPi=False
                raise RuntimeError("Unable to send data to " + GlobVar.address + ":" + str(GlobVar.port))
            totalsent = totalsent + sent
            
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
                self.receiveFlag.wait(timeout=GlobVar.timeout) # Max waiting time = GlobVar.timeout
                
                if self.receiveFlag.is_set():
                    ans=self.receivedAnswer
                    self.receivedAnswer=""
                else:
                    ans=self.ANSWER_TIMEOUT
                       
                self.receiveFlag.clear()
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
            return NetworkAnswers.ACK
        elif self.ANSWER_NACK in ans:
            return NetworkAnswers.NACK
        elif self.ANSWER_COM_ERROR in ans:
            return NetworkAnswers.COM_ERROR
        elif self.ANSWER_TIMEOUT in ans:
            return NetworkAnswers.TIMEOUT_ERROR
        elif self.ANSWER_CMD_REJECTED in ans:
            return NetworkAnswers.CMD_REJECTED
        else:
            return NetworkAnswers.UNKNOWN
    
    # Send OpenCom command to server
    def robotOpenCom(self) -> int:
        ans = self.__sendCommand(self.ROBOT_COM_OPEN, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send CloseCom command to server
    def robotCloseCom(self) -> int:
        ans = self.__sendCommand(self.ROBOT_COM_CLOSE, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send StartWithWatchdog command to server
    def robotStartWithWatchdog(self) -> int:
        ans = self.__sendCommand(self.ROBOT_START_WITH_WD, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send StartWithoutWatchdog command to server
    def robotStartWithoutWatchdog(self) -> int:
        ans = self.__sendCommand(self.ROBOT_START_WITHOUT_WD, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send Reset command to server
    def robotReset(self) -> int:
        ans = self.__sendCommand(self.ROBOT_RESET, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send Stop command to server
    def robotStop(self) -> int:
        ans = self.__sendCommand(self.ROBOT_STOP, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send GoLeft command to server
    def robotGoLeft(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_LEFT, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send GoRight command to server
    def robotGoRight(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_RIGHT, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send GoForward command to server
    def robotGoForward(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_FORWARD, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send GoBackward command to server
    def robotGoBackward(self) -> int:
        ans = self.__sendCommand(self.ROBOT_GO_BACKWARD, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send GetBattery command to server
    def robotGetBattery(self) -> None:
        ans = self.__sendCommand(self.ROBOT_GET_BATTERY,False)
        
    # Send OpenCamera command to server
    def cameraOpen(self) -> int:
        ans = self.__sendCommand(self.CAMERA_OPEN, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send CloseCamera command to server
    def cameraClose(self) -> int:
        ans = self.__sendCommand(self.CAMERA_CLOSE, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send GetPosition command to server
    def cameraGetPosition(self) -> int:
        ans = self.__sendCommand(self.CAMERA_POSITION_COMPUTE, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send StopPosition command to server
    def cameraStopPosition(self) -> int:
        ans = self.__sendCommand(self.CAMERA_POSITION_STOP, True)
        decodedAns = self.__decodeAnswer(ans)
        self.answerEvent.emit(decodedAns)
        return decodedAns
    
    # Send AskArena command to server
    def cameraAskArena(self) -> None:
        ans = self.__sendCommand(self.CAMERA_ARENA_ASK,False)

    # Send ConfirmArena command to server
    def cameraConfirmArena(self) -> None:
        ans = self.__sendCommand(self.CAMERA_ARENA_CONFIRM,False)
        
    # Send InfirmArena command to server
    def cameraInfirmArena(self) -> None:
        ans = self.__sendCommand(self.CAMERA_ARENA_INFIRM,False)
    
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


