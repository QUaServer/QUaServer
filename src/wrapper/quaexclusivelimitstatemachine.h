#ifndef QUAEXCLUSIVELIMITSTATEMACHINE_H
#define QUAEXCLUSIVELIMITSTATEMACHINE_H

#include <QUaFiniteStateMachine>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 9 - 5.8.12.2
/*
The ExclusiveLimitStateMachineType defines the state machine used by AlarmConditionTypes
that handle multiple mutually exclusive limits.

The ExclusiveLimitStateMachineType defines the sub state machine that represents the actual
level of a multilevel Alarm when it is in the Active state. The sub state machine defined here
includes High, Low, HighHigh and LowLow states. This model also includes in its transition
state a series of transition to and from a parent state, the inactive state. This state machine as
it is defined shall be used as a sub state machine for a state machine which has an Active state.
This Active state could be part of a “level” Alarm or “deviation” Alarm or any other Alarm state
machine.

The LowLow, Low, High, HighHigh are typical for many industries. Vendors can introduce substate 
models that include additional limits; they may also omit limits in an instance. If a model
omits states or transitions in the StateMachine, it is recommended that they provide the optional
Property AvailableStates and/or AvailableTransitions (see OPC 10000-5).

HasComponent | Object | HighHigh       | StateType
HasComponent | Object | High           | StateType
HasComponent | Object | Low            | StateType
HasComponent | Object | LowLow         | StateType
HasComponent | Object | LowToLowLow    | TransitionType
HasComponent | Object | LowLowToLow    | TransitionType
HasComponent | Object | HighToHighHigh | TransitionType
HasComponent | Object | HighHighToHigh | TransitionType

Transitions :

BrowseName     | References | BrowseName         | TypeDefinition
------------------------------------------------------------
HighHighToHigh | FromState  | HighHigh           | StateType
               | ToState    | High               | StateType
               | HasEffect  | AlarmConditionType | 
HighToHighHigh | FromState  | High               | StateType
               | ToState    | HighHigh           | StateType
               | HasEffect  | AlarmConditionType | 
LowLowToLow    | FromState  | LowLow             | StateType
               | ToState    | Low                | StateType
               | HasEffect  | AlarmConditionType | 
LowToLowLow    | FromState  | Low                | StateType
               | ToState    | LowLow             | StateType
               | HasEffect  | AlarmConditionType | 

*/

class QUaExclusiveLimitStateMachine : public QUaFiniteStateMachine
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaExclusiveLimitStateMachine(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELIMITSTATEMACHINE_H

