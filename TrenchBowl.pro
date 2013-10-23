#-------------------------------------------------
#
# Project created by QtCreator 2013-09-15T22:06:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TrenchBowl
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11 -pedantic -Wall -Werror

SOURCES += main.cpp\
        mainwindow.cpp \
    playerthread.cpp \
    playlistwidget.cpp \
    waveformwidget.cpp \
    waveformthread.cpp

HEADERS  += mainwindow.h \
    playerthread.h \
    playlistwidget.h \
    waveformwidget.h \
    waveformthread.h

FORMS    += mainwindow.ui

LIBS += -lgroove
