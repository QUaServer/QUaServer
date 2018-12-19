QT += core
QT -= gui

CONFIG += c++11

TARGET = WrapperTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

include($$PWD/../../src/wrapper/qopcuaserver.pri)