# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/log_dialog.ui'
#
# Created by: PyQt5 UI code generator 5.14.1
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(510, 336)
        self.verticalLayout = QtWidgets.QVBoxLayout(Dialog)
        self.verticalLayout.setObjectName("verticalLayout")
        self.plainTextEdit = QtWidgets.QPlainTextEdit(Dialog)
        self.plainTextEdit.setReadOnly(True)
        self.plainTextEdit.setPlainText("")
        self.plainTextEdit.setObjectName("plainTextEdit")
        self.verticalLayout.addWidget(self.plainTextEdit)
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.pushButton_clearLog = QtWidgets.QPushButton(Dialog)
        self.pushButton_clearLog.setObjectName("pushButton_clearLog")
        self.horizontalLayout.addWidget(self.pushButton_clearLog)
        self.pushButton_closeLog = QtWidgets.QPushButton(Dialog)
        self.pushButton_closeLog.setObjectName("pushButton_closeLog")
        self.horizontalLayout.addWidget(self.pushButton_closeLog)
        self.verticalLayout.addLayout(self.horizontalLayout)

        self.retranslateUi(Dialog)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Messages Log"))
        self.pushButton_clearLog.setText(_translate("Dialog", "Clear log"))
        self.pushButton_closeLog.setText(_translate("Dialog", "Close"))
