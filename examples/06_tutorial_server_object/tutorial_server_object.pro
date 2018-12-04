TEMPLATE  = app
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   -= qt

include($$PWD/../../open62541.pri)

SOURCES += tutorial_server_object.c
