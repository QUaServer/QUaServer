#ifndef QUASTATEMACHINE_H
#define QUASTATEMACHINE_H

#include <QUaBaseObject>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaStateVariable;
class QUaTransitionVariable;

// Part 5 - B.4.2
/*
The StateMachineType is the base ObjectType for all StateMachineTypes. It defines a single
Variable which represents the current state of the machine. An instance of this ObjectType shall
generate an Event whenever a significant state change occurs. The Server decides which state
changes are significant. Servers shall use the GeneratesEvent ReferenceType to indicate which
Event(s) could be produced by the StateMachine.

Subtypes may add Methods which affect the state of the machine. The Executable Attribute is
used to indicate whether the Method is valid given the current state of the machine. The
generation of AuditEvents for Methods is defined in OPC 10000-4. A StateMachine may not be
active. In this case, the CurrentState and LastTransition Variables shall have a status equal to
Bad_StateNotActive (see Table B.17).

Subtypes may add components which are instances of StateMachineTypes. These components
are considered to be sub-states of the StateMachine. SubStateMachines are only active when
the parent machine is in an appropriate state.

Events produced by SubStateMachines may be suppressed by the parent machine. In some
cases, the parent machine will produce a single Event that reflects changes in multiple
SubStateMachines.

FiniteStateMachineType is subtype of StateMachineType that provides a mechanism to
explicitly define the states and transitions. A Server should use this mechanism if it knows what
the possible states are and the state machine is not trivial. The FiniteStateMachineType is
defined in B.4.5.

HasComponent | Variable | CurrentState   | LocalizedText | StateVariableType      | Mandatory
HasComponent | Variable | LastTransition | LocalizedText | TransitionVariableType | Optional

*/

class QUaStateMachine : public QUaBaseObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaStateMachine(
		QUaServer* server
	);

	// CurrentState stores the current state of an instance of the StateMachineType. CurrentState
	// provides a human readable name for the current state which may not be suitable for use in
	// application control logic.Applications should use the Id Property of CurrentState if they need a
	// unique identifier for the state.
	QUaLocalizedText currentState() const;
	void setCurrentState(const QUaLocalizedText& currentState);

	// LastTransition stores the last transition which occurred in an instance of the StateMachineType.
	// LastTransition provides a human readable name for the last transition which may not be suitable
	// for use in application control logic.Applications should use the Id Property of LastTransition if
	// they need a unique identifier for the transition.
	// NOTE : optional; not created until one of these methods is called
	QUaLocalizedText lastTransition() const;
	void setLastTransition(const QUaLocalizedText& lastTransition);

protected:
	// LocalizedText
	QUaStateVariable* getCurrentState();
	// LocalizedText
	QUaTransitionVariable* getLastTransition();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUASTATEMACHINE_H

