#!/usr/bin/env python3

from email.message import Message
from email.policy import Policy
import os

import time
import sys
from tkinter import Image

from PyQt5 import (QtCore, QtWidgets, QtGui)

from main_window import Ui_MainWindow
from log_dialog import Ui_Dialog
from network import *
from globvar import GlobVar

import base64

class Window(QtWidgets.QMainWindow, Ui_MainWindow):
    _msg_dialog= None
    _batteryTimer=None
    _FPSTimer=None
    
    fps=0
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupUi(self)
        
        #self.DisableUIWidgets("Network")
        
        self.lineEdit_address.setText(GlobVar.address)
        self.lineEdit_port.setText(str(GlobVar.port))
        
        self._msg_dialog=QtWidgets.QDialog()
        self._msg_dialog.ui= Ui_Dialog()
        self._msg_dialog.ui.setupUi(self._msg_dialog)
        
        self.fps=0
        
        self.networkThread = Network()
        
        self.connectSignalSlots()
        
        # Start network thread and wait for event indicating monitor is connected to a target
        self.networkThread.start()
        
        # Create battery timer
        self._batteryTimer = QtCore.QTimer()
        self._batteryTimer.timeout.connect(self.OnBatteryTimeout)
        
        # Create fps timer
        self._FPSTimer = QtCore.QTimer()
        self._FPSTimer.timeout.connect(self.OnFPSTimeout)
        
    def connectSignalSlots(self):
        # Buttons
        self.pushButton_start.pressed.connect(self.OnButtonPress_Start)
        self.pushButton_confirmArena.pressed.connect(self.OnButtonPress_ConfirmArena)
        self.pushButton_up.pressed.connect(self.OnButtonPress_Up)
        self.pushButton_down.pressed.connect(self.OnButtonPress_Down)
        self.pushButton_stop.pressed.connect(self.OnButtonPress_Stop)
        self.pushButton_left.pressed.connect(self.OnButtonPress_Left)
        self.pushButton_right.pressed.connect(self.OnButtonPress_Right)
        
        # Checkbox
        self.checkBox_enableCamera.stateChanged.connect(self.OnCheckBoxChanged_EnableCamera)
        self.checkBox_enableFPS.stateChanged.connect(self.OnCheckBoxChanged_EnableFPS)
        self.checkBox_enablePosition.stateChanged.connect(self.OnCheckBoxChanged_EnablePosition)
        self.checkBox_getBattery.stateChanged.connect(self.OnCheckBoxChanged_GetBattery)
        
        # LineEdit
        self.lineEdit_address.textChanged.connect(self.OnLineEditChange_Address)
        self.lineEdit_port.textChanged.connect(self.OnLineEditChange_Port)
        
        # Menu
        self.action_OpenMessageLog.triggered.connect(self.OnMenu_OpenMessageLog)
        self.action_Quitter.triggered.connect(self.OnMenu_Quitter)
        
        # Message Dialog
        self._msg_dialog.ui.pushButton_clearLog.pressed.connect(self.OnButtonPress_ClearLog)
        self._msg_dialog.ui.pushButton_closeLog.pressed.connect(self.OnButtonPress_CloseLog)
        
        # Network signals
        self.networkThread.receptionEvent.connect(self.OnReceptionEvent)
        self.networkThread.connectionEvent.connect(self.OnConnectionEvent)
        self.networkThread.logEvent.connect(self.OnLogEvent)
        self.networkThread.answerEvent.connect(self.OnAnswerEvent)
    
    def EnableUIWidgets(self, area):
        if area == "Network":
            self.groupBox_activation.setDisabled(False)
            self.label_Image.setDisabled(False)
            self.pushButton_confirmArena.setDisabled(False)
            self.checkBox_enableCamera.setDisabled(False)
            self.checkBox_enableFPS.setDisabled(False)
            self.checkBox_enablePosition.setDisabled(False)
        else:
            self.groupBox_mouvments.setDisabled(False)
            self.groupBox_AnswerandBattery.setDisabled(False)  
                      
    def DisableUIWidgets(self, area):
        if area == "Network":
            self.groupBox_activation.setDisabled(True)
            self.label_Image.setDisabled(True)
            self.pushButton_confirmArena.setDisabled(True)
            self.checkBox_enableCamera.setDisabled(True)
            self.checkBox_enableFPS.setDisabled(True)
            self.checkBox_enablePosition.setDisabled(True)
            self.groupBox_AnswerandBattery.setDisabled(True)
            self.groupBox_mouvments.setDisabled(True)
        else:
            self.groupBox_mouvments.setDisabled(True)
            self.groupBox_AnswerandBattery.setDisabled(True)
            
    @QtCore.pyqtSlot(str)  
    def OnLineEditChange_Address(self, text):
        if self.networkThread.checkAddressIsValid(text):
            GlobVar.address = text
        
    @QtCore.pyqtSlot(str)  
    def OnLineEditChange_Port(self, text):
        GlobVar.port = int(text)
    
    @QtCore.pyqtSlot() 
    def OnButtonPress_Start (self):
        if self.pushButton_start.text() == "Start r&obot":
            if self.networkThread.robotOpenCom() == NetworkAnswers.ACK:
                # com opened successfully, start robot
                if self.checkBox_watchdog.isChecked():
                    # start with watchdog
                    if self.networkThread.robotStartWithWatchdog() == NetworkAnswers.ACK:
                        self.pushButton_start.setText("Reset r&obot")
                        self.EnableUIWidgets("Robot")
                else:
                     # start without watchdog
                    if self.networkThread.robotStartWithoutWatchdog() == NetworkAnswers.ACK:
                        self.pushButton_start.setText("Reset r&obot")
                        self.EnableUIWidgets("Robot")
        else:
            if self.networkThread.robotCloseCom() != NetworkAnswers.ACK:
                msg= QtWidgets.QMessageBox
                msg.warning(self,'Invalid answer', 'Server answer was not acknowledged. Maybe robot is still running', msg.Ok)
            
            self.pushButton_start.setText("Start r&obot")
            #self.DisableUIWidgets("Robot")
            
    @QtCore.pyqtSlot() 
    def OnButtonPress_ConfirmArena(self):
        self.networkThread.cameraAskArena()
        msg= QtWidgets.QMessageBox
        ret = msg.question(self, '', 'Arena boundaries are correctly detected ?',msg.Yes| msg.No)
        
        if ret == msg.Yes:
            self.networkThread.cameraConfirmArena()
            print ("Answer is YES")
        else:
            self.networkThread.cameraInfirmArena()
            print ("Answer is NO")
        
    @QtCore.pyqtSlot() 
    def OnButtonPress_Up(self):
        self.networkThread.robotGoForward()
      
    @QtCore.pyqtSlot()   
    def OnButtonPress_Down(self):
        self.networkThread.robotGoBackward()
    
    @QtCore.pyqtSlot() 
    def OnButtonPress_Stop(self):
        self.networkThread.robotStop()
     
    @QtCore.pyqtSlot() 
    def OnButtonPress_Left(self):
        self.networkThread.robotGoLeft()
     
    @QtCore.pyqtSlot() 
    def OnButtonPress_Right(self):
        self.networkThread.robotGoRight()
      
    @QtCore.pyqtSlot(int)  
    def OnCheckBoxChanged_EnableCamera(self, state):
        if self.checkBox_enableCamera.isChecked():
            self.networkThread.cameraOpen()
        else:
            self.networkThread.cameraClose()
        
    @QtCore.pyqtSlot(int) 
    def OnCheckBoxChanged_EnableFPS(self, state):
        if state !=0:
            self._FPSTimer.start(1000)
            self.checkBox_enableFPS.setText("FPS (0)")
        else:
            self._FPSTimer.stop()
            self.checkBox_enableFPS.setText("Enable FPS")
        
    @QtCore.pyqtSlot(int) 
    def OnCheckBoxChanged_EnablePosition(self, state):
        if self.checkBox_enablePosition.isChecked():
            self.networkThread.cameraGetPosition()
        else:
            self.networkThread.cameraStopPosition()
        
    @QtCore.pyqtSlot(int) 
    def OnCheckBoxChanged_GetBattery(self, state):
        if state !=0:
            self._batteryTimer.start(5000)
        else:
            self._batteryTimer.stop()
        
    @QtCore.pyqtSlot() 
    def OnMenu_OpenMessageLog(self):
        self._msg_dialog.show()
       
    @QtCore.pyqtSlot()  
    def OnMenu_Quitter(self):
        self._msg_dialog.hide()
        self.close()
          
    def closeEvent(self, event):
        self._msg_dialog.hide()
        event.accept()
          
    @QtCore.pyqtSlot() 
    def OnButtonPress_ClearLog(self):
        plainTextEdit=self._msg_dialog.ui.plainTextEdit
        plainTextEdit.document().clear()
        
    @QtCore.pyqtSlot() 
    def OnButtonPress_CloseLog(self):
        self._msg_dialog.hide()
        
    # Callback used to decode non answer message from server (mainly battery level and log message)
    @QtCore.pyqtSlot(str) 
    def OnReceptionEvent(self, s) -> None:   
        if Network.ROBOT_BATTERY_LEVEL in s:
            str_split = s.split(':')
        
            try:
                batteryLevel = int(str_split[1])
            except:
                batteryLevel = -1
    
            if  batteryLevel == 0:
                self.checkBox_getBattery.setText ("Get battery (0 = empty)")
            elif batteryLevel == 1:
                self.checkBox_getBattery.setText ("Get battery (1 = low)")
            elif batteryLevel == 2:
                self.checkBox_getBattery.setText ("Get battery (2 = full)")
            else:
                self.checkBox_getBattery.setText ("Get battery (invalid value)")
        elif Network.CAMERA_IMAGE in s:
            #print ("Image received")
            #print ("Date received: " + s)
            
            self.fps=self.fps+1
            str_split = s.split(':')
            try:
                image_jpg= base64.b64decode(str_split[1])
                img = QtGui.QImage.fromData(image_jpg, "jpg")
                im_pixmap = QtGui.QPixmap(QtGui.QPixmap.fromImage(img))
            
                #print ("Image size: " + str(im_pixmap.width()) + "x" + str(im_pixmap.height()))
            
                self.label_Image.setPixmap(im_pixmap)
                self.label_Image.setScaledContents(True)
                self.label_Image.setSizePolicy(QtWidgets.QSizePolicy.Fixed,QtWidgets.QSizePolicy.Fixed)
            except:
                pass
                #print ("Invalid image received")
        elif Network.CAMERA_POSITION in s:          
            #CPOS:-1;0.000000;0.000000;0.000000;0.000000;0.000000
            str_split = s.split(':')
            values_split = str_split[1].split(';')
            
            try: 
                robot_ID = int(values_split[0]) # Id of robot
            except:
                robot_ID = -1
            
            try:
                robot_Angle = float(values_split[1]) # angle of robot
            except:
                robot_Angle = 0.0
                
            try:
                robot_Coord_X = float(values_split[2]) # X coord of robot
            except:
                robot_Coord_X = 0.0
                
            try:
                robot_Coord_Y = float(values_split[3]) # Y coord of robot
            except:
                robot_Coord_Y = 0.0
                
            try:
                robot_Cap_X = float(values_split[4]) # X cap of robot
            except:
                robot_Cap_X = 0.0
                
            try:
                robot_Cap_Y = float(values_split[5]) # Y cap of robot
            except:
                robot_Cap_Y = 0.0
            
            if robot_ID == -1:
                self.label_RobotID.setText("No robot (-1)")
            else:
                self.label_RobotID.setText(values_split[0])
            
            self.label_RobotAngle.setText("%.2f°" % (robot_Angle))
            self.label_RobotPos.setText("(%.2f, %.2f)" % (robot_Coord_X, robot_Coord_Y))
            self.label_RobotDirection.setText("(%.2f, %.2f)" % (robot_Cap_X, robot_Cap_Y))
              
    # Callback for battery timeout
    @QtCore.pyqtSlot() 
    def OnBatteryTimeout(self) -> None:
        # Send a request for battery level. Answer will be done in OnReceptionEvent callback
        self.networkThread.robotGetBattery()
    
    # Callback for FPS timeout
    @QtCore.pyqtSlot() 
    def OnFPSTimeout(self) -> None:
        # Display current FPS
        self.checkBox_enableFPS.setText("FPS (" + str(self.fps)+")")
        self.fps=0
        
    # Callback for connection/deconnection event from network manager
    @QtCore.pyqtSlot(int) 
    def OnConnectionEvent(self, event) -> None:       
        if event == NetworkEvents.EVENT_CONNECTED:
            GlobVar.connectedToPi = True
            print ("Connected to server")
            
            self.label_connectionStatus.setText("Connected")
            self.EnableUIWidgets("Network")
        elif event == NetworkEvents.EVENT_CONNECTION_LOST:
            GlobVar.connectedToPi = False
            print ("Disconnected from server")
            
            self.label_connectionStatus.setText("Not connected")
            self.pushButton_start.setText("Start r&obot")
            #self.DisableUIWidgets("Network")
            
    # Callback for answer event from network manager
    @QtCore.pyqtSlot(int) 
    def OnAnswerEvent(self, ans) -> None:
        if ans == NetworkAnswers.ACK:
            self.label_lastAnswer.setText("Acknowledged (AACK)")
        elif ans == NetworkAnswers.NACK:
            self.label_lastAnswer.setText("Not acknowledged (ANAK)")
        elif ans == NetworkAnswers.COM_ERROR:
            self.label_lastAnswer.setText("Command error (ACER)")
        elif ans == NetworkAnswers.TIMEOUT_ERROR:
            self.label_lastAnswer.setText("Timeout - no answer")
        elif ans == NetworkAnswers.CMD_REJECTED:
            self.label_lastAnswer.setText("Command rejected (ACRJ)")
        else:
            self.label_lastAnswer.setText("Unknown answer")
    
    # Callback for log event from network manager
    @QtCore.pyqtSlot(str) 
    def OnLogEvent(self, txt) -> None:
        self._msg_dialog.ui.plainTextEdit.textCursor().insertText(txt)
    
try: 
    if len(sys.argv)>=2:
        GlobVar.address = sys.argv[1]
    else:
        print ("No target address specified: using localhost:5544")
        print ("Usage: monitor-python.py address [port]")
        
        #exit (-1) # Comment this line for connecting to localhost
        
    if len(sys.argv)>=3:
        GlobVar.port = int(sys.argv[2])
        
    app = QtWidgets.QApplication(sys.argv)
    window = Window()
    window.show()
    app.exec_()
    
except KeyboardInterrupt: # exception when pressing CTRL-C
    print ("Bye bye")

