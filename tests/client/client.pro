QT     += core
CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

include($$PWD/../../open62541.pri)
include($$PWD/../../common/QConsoleListener.git/src/qconsolelistener.pri)
include($$PWD/../../common/QDeferred.git/src/qlambdathreadworker.pri)

INCLUDEPATH += $$PWD

SOURCES += main.cpp

include($$PWD/../../common/QConsoleListener.git/src/add_qt_path_win.pri)