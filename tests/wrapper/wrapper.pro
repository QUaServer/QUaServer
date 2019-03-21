QT += core
QT -= gui

CONFIG += c++11

TARGET = WrapperTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

SOURCES += \   
    $$PWD/mynewobjecttype.cpp \
    $$PWD/mynewvariabletype.cpp

HEADERS += \
    $$PWD/mynewobjecttype.h \
    $$PWD/mynewvariabletype.h

include($$PWD/../../src/wrapper/quaserver.pri)
include($$PWD/../../common/add_qt_path_win.pri)