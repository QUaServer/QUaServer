QT += core sql
QT -= gui

CONFIG += c++11

TARGET = 10_historizing
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += \
main.cpp \
quainmemoryhistorizer.cpp \
quasqlitehistorizer.cpp

HEADERS += \
quainmemoryhistorizer.h \
quasqlitehistorizer.h

ua_events || ua_alarms_conditions {
	SOURCES += \
	$$PWD/../08_events/myevent.cpp
	HEADERS += \
	$$PWD/../08_events/myevent.h
}

include($$PWD/../../src/wrapper/quaserver.pri)
include($$PWD/../../src/helper/add_qt_path_win.pri)