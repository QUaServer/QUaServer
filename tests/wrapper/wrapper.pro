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
include($$PWD/../../common/add_qt_path_win.pri)