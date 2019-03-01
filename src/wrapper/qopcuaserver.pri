include($$PWD/../open62541.pri)

QT     += core
CONFIG += c++11

INCLUDEPATH += $$PWD/

SOURCES += \
    $$PWD/qopcuaserver.cpp \
    $$PWD/qopcuaservernode.cpp \
    $$PWD/qopcuabasevariable.cpp \
    $$PWD/qopcuaproperty.cpp \
    $$PWD/qopcuabasedatavariable.cpp \
    $$PWD/qopcuabaseobject.cpp \
    $$PWD/qopcuafolderobject.cpp

SOURCES += \   
    $$PWD/qopcuatypesconverter.cpp

HEADERS += \
    $$PWD/qopcuaserver.h \
    $$PWD/qopcuaservernode.h \
    $$PWD/qopcuabasevariable.h \
    $$PWD/qopcuaproperty.h \
    $$PWD/qopcuabasedatavariable.h \
    $$PWD/qopcuabaseobject.h \
    $$PWD/qopcuafolderobject.h

HEADERS += \    
    $$PWD/qopcuatypesconverter.h

DISTFILES += \
    $$PWD/QOpcUaServer \
    $$PWD/QOpcUaServerNode \
    $$PWD/QOpcUaBaseVariable \
    $$PWD/QOpcUaProperty \
    $$PWD/QOpcUaBaseDataVariable \
    $$PWD/QOpcUaBaseObject \
    $$PWD/QOpcUaFolderObject \

DISTFILES += \    
    $$PWD/QOpcUaTypesConverter
