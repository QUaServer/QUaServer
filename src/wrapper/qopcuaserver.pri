include($$PWD/../open62541.pri)

INCLUDEPATH += $$PWD/

SOURCES += \
    $$PWD/qopcuaserver.cpp \
    $$PWD/qopcuabasedatavariable.cpp \
    $$PWD/qopcuabaseobject.cpp \
    $$PWD/qopcuafolderobject.cpp \
    $$PWD/qopcuaservernode.cpp \
    $$PWD/qopcuabasevariable.cpp

HEADERS += \
    $$PWD/qopcuaserver.h \
    $$PWD/qopcuabasedatavariable.h \
    $$PWD/qopcuabaseobject.h \
    $$PWD/qopcuafolderobject.h \
    $$PWD/qopcuaservernode.h \
    $$PWD/qopcuabasevariable.h

DISTFILES += \
    $$PWD/QOpcUaServerNode \
    $$PWD/QOpcUaBaseObject \
    $$PWD/QOpcUaServer \
    $$PWD/QOpcUaBaseVariable \
    $$PWD/QOpcUaServerNodeFactory \
    $$PWD/QOpcUaFolderObject
