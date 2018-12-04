TEMPLATE = lib
CONFIG  += staticlib
CONFIG  -= app_bundle
CONFIG  -= qt

DESTDIR  = $$PWD/build

CONFIG(debug, debug|release) {
	OBJECTS_DIR = $$PWD/build/debug
	TARGET = open62541d
} else {
	OBJECTS_DIR = $$PWD/build/release
	TARGET = open62541
}	

INCLUDEPATH += $$PWD/src

SOURCES += $$PWD/src/open62541.c