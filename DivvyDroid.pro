#-------------------------------------------------
#
# Project created by QtCreator 2019-05-06T12:13:40
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DivvyDroid
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        device/adbclient.cpp \
        device/deviceinfo.cpp \
        device/videothread.cpp \
        input/devicebuttonhandler.cpp \
        input/devicetouchhandler.cpp \
        input/input_to_adroid_keys.cpp \
        input/inputhandler.cpp \
        input/shellkeyboardhandler.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        device/adbclient.h \
        device/deviceinfo.h \
        device/videothread.h \
        input/android_keycodes.h \
        input/devicebuttonhandler.h \
        input/devicetouchhandler.h \
        input/input_event_codes.h \
        input/input_to_adroid_keys.h \
        input/inputhandler.h \
        input/shellkeyboardhandler.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    divvydroid.qrc

DISTFILES +=
