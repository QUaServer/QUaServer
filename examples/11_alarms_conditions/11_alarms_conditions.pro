QT += core
QT -= gui

CONFIG += c++11

TARGET = 11_alarms_conditions
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

ua_historizing {
	QT += sql
	SOURCES += \
	$$PWD/../10_historizing/quainmemoryhistorizer.cpp \
	$$PWD/../10_historizing/quasqlitehistorizer.cpp
	HEADERS += \
	$$PWD/../10_historizing/quainmemoryhistorizer.h \
	$$PWD/../10_historizing/quasqlitehistorizer.h
}

include($$PWD/../../src/wrapper/quaserver.pri)
include($$PWD/../../src/helper/add_qt_path_win.pri)