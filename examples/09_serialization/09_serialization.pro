QT += core
QT -= gui

CONFIG += c++11

TARGET = 09_serialization
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

HEADERS += temperaturesensor.h
SOURCES += temperaturesensor.cpp

include($$PWD/../../src/wrapper/quaserver.pri)
include($$PWD/../../src/helper/add_qt_path_win.pri)