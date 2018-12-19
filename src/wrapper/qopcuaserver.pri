include($$PWD/../open62541.pri)

INCLUDEPATH += $$PWD/

SOURCES += \
    $$PWD/qopcuaserver.cpp \
    $$PWD/qopcuabaseobjecttype.cpp \
    $$PWD/qopcuabasedatavariabletype.cpp \
    $$PWD/qopcuafolderobjecttype.cpp \
    $$PWD/qopcuabasedatavariable.cpp \
    $$PWD/qopcuabaseobject.cpp \
    $$PWD/qopcuafolderobject.cpp \
    $$PWD/qopcuaservernode.cpp \
    $$PWD/qopcuabasevariabletype.cpp

HEADERS += \
    $$PWD/qopcuaserver.h \
    $$PWD/qopcuabaseobjecttype.h \
    $$PWD/qopcuabasedatavariabletype.h \
    $$PWD/qopcuafolderobjecttype.h \
    $$PWD/qopcuabasedatavariable.h \
    $$PWD/qopcuabaseobject.h \
    $$PWD/qopcuafolderobject.h \
    $$PWD/qopcuaservernode.h \
    $$PWD/qopcuabasevariabletype.h

DISTFILES += \
    $$PWD/QOpcUaServerNode \
    $$PWD/QOpcUaBaseObjectType \
    $$PWD/QOpcUaBaseObject \
    $$PWD/QOpcUaBaseVariableType