include($$PWD/../amalgamation/open62541.pri)

QT     += core
CONFIG += c++11
CONFIG -= flat

INCLUDEPATH += $$PWD/

LANGUAGE = C++
CONFIG += precompile_header
PRECOMPILED_HEADER = $$PWD/pch_open62541.h

SOURCES += \
    $$PWD/quaserver.cpp \
    $$PWD/quanode.cpp \
    $$PWD/quabasevariable.cpp \
    $$PWD/quaproperty.cpp \
    $$PWD/quabasedatavariable.cpp \
    $$PWD/quabaseobject.cpp \
    $$PWD/quafolderobject.cpp \
    $$PWD/quacustomdatatypes.cpp

ua_events || ua_alarms_conditions {
    SOURCES += \
    $$PWD/quabaseevent.cpp \
    $$PWD/quageneralmodelchangeevent.cpp
}

ua_alarms_conditions {
    SOURCES += \
    $$PWD/quaconditionvariable.cpp \
    $$PWD/quastatevariable.cpp \
    $$PWD/quatwostatevariable.cpp \
    $$PWD/quacondition.cpp \
    $$PWD/quaacknowledgeablecondition.cpp \
    $$PWD/quaalarmcondition.cpp
}

ua_historizing {
    SOURCES += \
    $$PWD/quahistorybackend.cpp
}

SOURCES += \   
    $$PWD/quatypesconverter.cpp

HEADERS += \
    $$PWD/quaserver.h \
    $$PWD/quanode.h \
    $$PWD/quabasevariable.h \
    $$PWD/quaproperty.h \
    $$PWD/quabasedatavariable.h \
    $$PWD/quabaseobject.h \
    $$PWD/quafolderobject.h \
    $$PWD/quacustomdatatypes.h

ua_events || ua_alarms_conditions {
    HEADERS += \
    $$PWD/quabaseevent.h \
    $$PWD/quageneralmodelchangeevent.h
}

ua_alarms_conditions {
    HEADERS += \
    $$PWD/quaconditionvariable.h \
    $$PWD/quastatevariable.h \
    $$PWD/quatwostatevariable.h \
    $$PWD/quacondition.h \
    $$PWD/quaacknowledgeablecondition.h \
    $$PWD/quaalarmcondition.h
}

ua_historizing {
    HEADERS += \
    $$PWD/quahistorybackend.h
}
    
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
    $$PWD/QUaCustomDataTypes

ua_events || ua_alarms_conditions {
    DISTFILES += \
    $$PWD/QUaBaseEvent \
    $$PWD/QUaGeneralModelChangeEvent
}

ua_alarms_conditions {
    DISTFILES += \
    $$PWD/QUaConditionVariable \
    $$PWD/QUaStateVariable \
    $$PWD/QUaTwoStateVariable \
    $$PWD/QUaCondition \
    $$PWD/QUaAcknowledgeableCondition \
    $$PWD/QUaAlarmCondition
}

ua_historizing {
    DISTFILES += \
    $$PWD/QUaHistoryBackend
}

DISTFILES += \    
    $$PWD/QUaTypesConverter