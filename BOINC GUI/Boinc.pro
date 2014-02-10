#-------------------------------------------------
#
# Project created by QtCreator 2013-11-30T23:09:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Boinc
TEMPLATE = app


SOURCES += main.cpp\
        boincmanager.cpp \
    boincdialog.cpp

CONFIG += console

HEADERS  += boincmanager.h \
    boincdialog.h

FORMS    += boincmanager.ui \
    boincdialog.ui

OTHER_FILES += \
    boincnitc.png

RESOURCES += \
    Icon.qrc
