#-------------------------------------------------
#
# Project created by QtCreator 2018-06-27T09:28:50
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Blockchain
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11
CONFIG += fexceptions
CONFIG += exceptions
QMAKE_LFLAGS = -static
QMAKE_CXXFLAGS += -fexceptions

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dialog.cpp \
    client.cpp \
    server.cpp

HEADERS += \
        mainwindow.h \
    Block.h \
    Blockchain.h \
    File.h \
    server.h \
    dialog.h \
    client.h

FORMS += \
        mainwindow.ui
