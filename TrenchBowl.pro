#-------------------------------------------------
#
# Project created by QtCreator 2013-09-15T22:06:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TrenchBowl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

LIBS += -L$$PWD/libgroove/src/ -lgroove

INCLUDEPATH += $$PWD/libgroove/src
DEPENDPATH += $$PWD/libgroove/src

PRE_TARGETDEPS += $$PWD/libgroove/src/libgroove.a
