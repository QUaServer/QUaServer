include($$PWD/../amalgamation/open62541.pri)

QT     += core
CONFIG += c++11

INCLUDEPATH += $$PWD/

SOURCES += \
    $$PWD/quaserver.cpp \
    $$PWD/quanode.cpp \
    $$PWD/quabasevariable.cpp \
    $$PWD/quaproperty.cpp \
    $$PWD/quabasedatavariable.cpp \
    $$PWD/quabaseobject.cpp \
    $$PWD/quafolderobject.cpp

SOURCES += \   
    $$PWD/quatypesconverter.cpp

HEADERS += \
    $$PWD/quaserver.h \
    $$PWD/quanode.h \
    $$PWD/quabasevariable.h \
    $$PWD/quaproperty.h \
    $$PWD/quabasedatavariable.h \
    $$PWD/quabaseobject.h \
    $$PWD/quafolderobject.h

HEADERS += \    
    $$PWD/quatypesconverter.h

DISTFILES += \
    $$PWD/QUaServer \
    $$PWD/QUaNode \
    $$PWD/QUaBaseVariable \
    $$PWD/QUaProperty \
    $$PWD/QUaBaseDataVariable \
    $$PWD/QUaBaseObject \
    $$PWD/QUaFolderObject \

DISTFILES += \    
    $$PWD/QUaTypesConverter
