#ifndef QUAFINITESTATEMACHINE_H
#define QUAFINITESTATEMACHINE_H

#include <QUaStateMachine>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaBaseDataVariable;

// Part 5 - B.4.5 
/*
The FiniteStateMachineType is the base ObjectType for StateMachines that explicitly define
the possible States and Transitions. Once the States and Transitions are defined subtypes shall
not add new States and Transitions (see B.4.18). Subtypes may add causes or effects.

The States of the machine are represented with instances of the StateType ObjectType. Each
State shall have a BrowseName which is unique within the StateMachine and shall have a
StateNumber which shall also be unique across all States defined in the StateMachine. Be
aware that States in a SubStateMachine may have the same StateNumber or BrowseName as
States in the parent machine. A concrete subtype of FiniteStateMachineType shall define at
least one State.

A StateMachine may define one State which is an instance of the InitialStateType. This State
is the State that the machine goes into when it is activated.

The Transitions that may occur are represented with instances of the TransitionType. Each
Transition shall have a BrowseName which is unique within the StateMachine and may have a
TransitionNumber which shall also be unique across all Transitions defined in the StateMachine.

The initial State for a Transition is a StateType Object which is the target of a FromState
Reference. The final State for a Transition is a StateType Object which is the target of a ToState
Reference. The FromState and ToState References shall always be specified.

A Transition may produce an Event. The Event is indicated by a HasEffect Reference to a
subtype of BaseEventType. The StateMachineType shall have GeneratesEvent References to
the targets of a HasEffect Reference for each of its Transitions.

A FiniteStateMachineType may define Methods that cause a transition to occur. These Methods
are targets of HasCause References for each of the Transitions that may be triggered by the
Method. The Executable Attribute for a Method is used to indicate whether the current State of
the machine allows the Method to be called.

A FiniteStateMachineType may have sub-state-machines which are represented as instances
of StateMachineType ObjectTypes. Each State shall have a HasSubStateMachine Reference
to the StateMachineType Object which represents the child States. The SubStateMachine is
not active if the parent State is not active. In this case the CurrentState and LastTransition
Variables of the SubStateMachine shall have a status equal to Bad_StateNotActive (see Table
B.17).

In some Servers an instance of a StateMachine may restrict the States and / or Transitions that
are available. These restrictions may result from the internal design of the instance. For
example the StateMachine for an instrument’s limit alarm which only supports Hi and HiHi and
can not produce a Low or LowLow. An instance of a StateMachine may also dynamically change
the available States and/or Transitions based on its operating mode. For example when a piece
of equipment is in a maintenance mode the available States may be limited to some subset of
the States available during normal operation.

HasComponent | Variable | CurrentState         | LocalizedText | FiniteStateVariableType      | Mandatory
HasComponent | Variable | LastTransition       | LocalizedText | FiniteTransitionVariableType | Optional
HasComponent | Variable | AvailableStates      | NodeId[]      | BaseDataVariableType         | Optional
HasComponent | Variable | AvailableTransitions | NodeId[]      | BaseDataVariableType         | Optional
*/

class QUaFiniteStateMachine : public QUaStateMachine
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteStateMachine(
		QUaServer* server
	);

	// inherited

	// QUaLocalizedText currentState

	// QUaLocalizedText lastTransition

	// children

	// The AvailableStates Variable provides a NodeId list of the States that are present in the
	// StateMachine instance.The list may change during operation of the Server.
	QVector<QUaNodeId> availableStates() const;
	void setAvailableStates(const QVector<QUaNodeId>& availableStates);

	// The AvailableTransitions Variable provides a NodeId list of the Transitions that are present in
	// the StateMachine instance.The list may change during operation of the Server.
	QVector<QUaNodeId> availableTransitions() const;
	void setAvailableTransitions(const QVector<QUaNodeId>& availableTransitions);

protected:
	QUaBaseDataVariable* getAvailableStates();

	QUaBaseDataVariable* getAvailableTransitions();
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITESTATEMACHINE_H

