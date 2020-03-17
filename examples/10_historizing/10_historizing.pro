QT += core sql
QT -= gui

CONFIG += c++11

TARGET = 10_historizing
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

#include($$PWD/../../src/amalgamation/open62541.pri)
#
#INCLUDEPATH += $$PWD/../../depends/open62541.git/include
#INCLUDEPATH += $$PWD/../../depends/open62541.git/plugins/include
#
## NOTE : compile amalgamation with UA_ENABLE_HISTORIZING
#
SOURCES += main.cpp

include($$PWD/../../src/wrapper/quaserver.pri)
include($$PWD/../../src/helper/add_qt_path_win.pri)