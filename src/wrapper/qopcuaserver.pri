include($$PWD/../open62541.pri)

INCLUDEPATH += $$PWD/

SOURCES += \
    $$PWD/qopcuaserver.cpp \
    $$PWD/qopcuabasedatavariable.cpp \
    $$PWD/qopcuabaseobject.cpp \
    $$PWD/qopcuafolderobject.cpp \
    $$PWD/qopcuaservernode.cpp \
    $$PWD/qopcuabasevariable.cpp \
    $$PWD/qopcuaabstractobject.cpp \
    $$PWD/qopcuaabstractvariable.cpp \
    $$PWD/qopcuatypesconverter.cpp

HEADERS += \
    $$PWD/qopcuaserver.h \
    $$PWD/qopcuabasedatavariable.h \
    $$PWD/qopcuabaseobject.h \
    $$PWD/qopcuafolderobject.h \
    $$PWD/qopcuaservernode.h \
    $$PWD/qopcuabasevariable.h \
    $$PWD/qopcuanodefactory.h \
    $$PWD/qopcuaabstractobject.h \
    $$PWD/qopcuaabstractvariable.h \
    $$PWD/qopcuatypesconverter.h

DISTFILES += \
    $$PWD/QOpcUaServerNode \
    $$PWD/QOpcUaBaseObject \
    $$PWD/QOpcUaServer \
    $$PWD/QOpcUaBaseVariable \
    $$PWD/QOpcUaServerNodeFactory \
    $$PWD/QOpcUaFolderObject \
    $$PWD/QOpcUaNodeFactory \
    $$PWD/QOpcUaBaseDataVariable \
    $$PWD/QOpcUaAbstractVariable \
    $$PWD/QOpcUaAbstractObject \
    $$PWD/QOpcUaTypesConverter
