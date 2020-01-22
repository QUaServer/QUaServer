QT += core xml sql
QT -= gui

CONFIG += c++11

TARGET = 09_serialization
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp \
    quasqliteserializer.cpp \
    quaxmlserializer.cpp

HEADERS += temperaturesensor.h \
    quasqliteserializer.h \
    quaxmlserializer.h
SOURCES += temperaturesensor.cpp

include($$PWD/../../src/wrapper/quaserver.pri)
include($$PWD/../../src/helper/add_qt_path_win.pri)
