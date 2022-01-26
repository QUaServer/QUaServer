include($$PWD/../amalgamation/open62541.pri)

QT     += core
CONFIG += c++11
CONFIG -= flat

INCLUDEPATH += $$PWD/

# ignore some useless warnings on visual studio builds
win32 {
    contains(QMAKE_CC, cl) {
        QMAKE_CXXFLAGS_WARN_ON -= -w14005
        QMAKE_CXXFLAGS += -wd4005
    }
}

SOURCES += \
    $$PWD/quaserver.cpp \
    $$PWD/quaserver_anex.cpp \
    $$PWD/quanode.cpp \
    $$PWD/quabasevariable.cpp \
    $$PWD/quaproperty.cpp \
    $$PWD/quabasedatavariable.cpp \
    $$PWD/quabaseobject.cpp \
    $$PWD/quafolderobject.cpp \
    $$PWD/quacustomdatatypes.cpp \
    $$PWD/quaenum.cpp

ua_events || ua_alarms_conditions {
    SOURCES += \
    $$PWD/quabaseevent.cpp \
    $$PWD/quabasemodelchangeevent.cpp \
    $$PWD/quageneralmodelchangeevent.cpp \
    $$PWD/quasystemevent.cpp
}

ua_alarms_conditions {
    SOURCES += \
    $$PWD/quaconditionvariable.cpp \
    $$PWD/quastatevariable.cpp \
    $$PWD/quafinitestatevariable.cpp \
    $$PWD/quatwostatevariable.cpp \
    $$PWD/quatransitionvariable.cpp \
    $$PWD/quafinitetransitionvariable.cpp \
    $$PWD/quastatemachine.cpp \
    $$PWD/quafinitestatemachine.cpp \
    $$PWD/quastate.cpp \
    $$PWD/quatransition.cpp \
    $$PWD/quaexclusivelimitstatemachine.cpp \
    $$PWD/quacondition.cpp \
    $$PWD/quaacknowledgeablecondition.cpp \
    $$PWD/quaalarmcondition.cpp \
    $$PWD/quadiscretealarm.cpp \
    $$PWD/quaoffnormalalarm.cpp \
    $$PWD/qualimitalarm.cpp \
    $$PWD/quaexclusivelimitalarm.cpp \
    $$PWD/quaexclusivelevelalarm.cpp \
    $$PWD/quarefreshstartevent.cpp \
    $$PWD/quarefreshendevent.cpp \
    $$PWD/quarefreshrequiredevent.cpp \
    $$PWD/quatransitionevent.cpp
}

ua_historizing {
    SOURCES += \
    $$PWD/quahistorybackend.cpp
}

ua_namespace_full || ua_events || ua_alarms_conditions {
    SOURCES += \
    $$PWD/quaoptionset.cpp \
    $$PWD/quaoptionsetvariable.cpp
}

SOURCES += \   
    $$PWD/quatypesconverter.cpp

HEADERS += \
    $$PWD/quaserver.h \
    $$PWD/quaserver_anex.h \
    $$PWD/quanode.h \
    $$PWD/quabasevariable.h \
    $$PWD/quaproperty.h \
    $$PWD/quabasedatavariable.h \
    $$PWD/quabaseobject.h \
    $$PWD/quafolderobject.h \
    $$PWD/quacustomdatatypes.h \
    $$PWD/quaenum.h

ua_events || ua_alarms_conditions {
    HEADERS += \
    $$PWD/quabaseevent.h \
    $$PWD/quabasemodelchangeevent.h \
    $$PWD/quageneralmodelchangeevent.h \
    $$PWD/quasystemevent.h
}

ua_alarms_conditions {
    HEADERS += \
    $$PWD/quaconditionvariable.h \
    $$PWD/quastatevariable.h \
    $$PWD/quafinitestatevariable.h \
    $$PWD/quatwostatevariable.h \
    $$PWD/quatransitionvariable.h \
    $$PWD/quafinitetransitionvariable.h \
    $$PWD/quastatemachine.h \
    $$PWD/quafinitestatemachine.h \
    $$PWD/quastate.h \
    $$PWD/quatransition.h \
    $$PWD/quaexclusivelimitstatemachine.h \
    $$PWD/quacondition.h \
    $$PWD/quaacknowledgeablecondition.h \
    $$PWD/quaalarmcondition.h \
    $$PWD/quadiscretealarm.h \
    $$PWD/quaoffnormalalarm.h \
    $$PWD/qualimitalarm.h \
    $$PWD/quaexclusivelimitalarm.h \
    $$PWD/quaexclusivelevelalarm.h \
    $$PWD/quarefreshstartevent.h \
    $$PWD/quarefreshendevent.h \
    $$PWD/quarefreshrequiredevent.h \
    $$PWD/quatransitionevent.h
}

ua_historizing {
    HEADERS += \
    $$PWD/quahistorybackend.h
}
    
ua_namespace_full || ua_events || ua_alarms_conditions {
    HEADERS += \
    $$PWD/quaoptionset.h \
    $$PWD/quaoptionsetvariable.h
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
    $$PWD/QUaBaseModelChangeEvent \
    $$PWD/QUaGeneralModelChangeEvent \
    $$PWD/QUaSystemEvent
}

ua_alarms_conditions {
    DISTFILES += \
    $$PWD/QUaConditionVariable \
    $$PWD/QUaStateVariable \
    $$PWD/QUaFiniteStateVariable \
    $$PWD/QUaTwoStateVariable \
    $$PWD/QUaTransitionVariable \
    $$PWD/QUaFiniteTransitionVariable \
    $$PWD/QUaStateMachine \
    $$PWD/QUaFiniteStateMachine \
    $$PWD/QUaState \
    $$PWD/QUaTransition \
    $$PWD/QUaExclusiveLimitStateMachine \
    $$PWD/QUaCondition \
    $$PWD/QUaAcknowledgeableCondition \
    $$PWD/QUaAlarmCondition \
    $$PWD/QUaDiscreteAlarm \
    $$PWD/QUaOffNormalAlarm \
    $$PWD/QUaLimitAlarm \
    $$PWD/QUaExclusiveLimitAlarm \
    $$PWD/QUaExclusiveLevelAlarm \
    $$PWD/QUaRefreshStartEvent \
    $$PWD/QUaRefreshEndEvent \
    $$PWD/QUaRefreshRequiredEvent \
    $$PWD/QUaTransitionEvent
}

ua_historizing {
    DISTFILES += \
    $$PWD/QUaHistoryBackend
}

ua_namespace_full || ua_events || ua_alarms_conditions {
    DISTFILES += \
    $$PWD/QUaOptionSet \
    $$PWD/QUaOptionSetVariable
}

DISTFILES += \    
    $$PWD/QUaTypesConverter
