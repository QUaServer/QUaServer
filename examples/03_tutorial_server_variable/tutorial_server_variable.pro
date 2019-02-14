TEMPLATE  = app
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   -= qt

include($$PWD/../../src/open62541.pri)

SOURCES += tutorial_server_variable.c
