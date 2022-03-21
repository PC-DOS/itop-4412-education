#-------------------------------------------------
#
# Project created by QtCreator 2022-03-21T12:05:12
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TCPNetworkDemo4412
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    NetworkingControlInterface.Client.cpp \
    NetworkingControlInterface.Server.cpp \
    SettingsProvider.cpp

HEADERS  += MainWindow.h \
    NetworkingControlInterface.Client.h \
    NetworkingControlInterface.h \
    NetworkingControlInterface.Server.h \
    SettingsProvider.h

FORMS    += MainWindow.ui
