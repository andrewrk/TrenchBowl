#-------------------------------------------------
#
# Project created by QtCreator 2013-09-15T22:06:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TrenchBowl
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    playerthread.cpp \
    playlistwidget.cpp

HEADERS  += mainwindow.h \
    playerthread.h \
    playlistwidget.h

FORMS    += mainwindow.ui


LIBS += -L$$PWD/libgroove/src/ -lgroove

INCLUDEPATH += $$PWD/libgroove/src
DEPENDPATH += $$PWD/libgroove/src


LIBS += $$PWD/libgroove/deps/libav/out/lib/libavfilter.a
LIBS += $$PWD/libgroove/deps/libav/out/lib/libavformat.a
LIBS += $$PWD/libgroove/deps/libav/out/lib/libavcodec.a
LIBS += $$PWD/libgroove/deps/libav/out/lib/libavresample.a
LIBS += $$PWD/libgroove/deps/libav/out/lib/libswscale.a
LIBS += $$PWD/libgroove/deps/libav/out/lib/libavutil.a

LIBS += $$PWD/libgroove/deps/ebur128/build/libebur128.a

LIBS += -lSDL -lbz2 -lz -lm -pthread
