#ifndef QUAFINITETRANSITIONVARIABLE_H
#define QUAFINITETRANSITIONVARIABLE_H

#include <QUaTransitionVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.7
/*
The FiniteTransitionVariableType is a subtype of TransitionVariableType and is used to store a
Transition that occurred within a FiniteStateMachine as a human readable name.

*/

class QUaFiniteTransitionVariable : public QUaTransitionVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteTransitionVariable(
		QUaServer* server
	);

	// inherited

	// Id is inherited from the TransitionVariableTypeand overridden to reflect the required DataType.
	// This value shall be the NodeId of one of the Transition Objects of the FiniteStateMachineType.
	QUaNodeId id() const;
	void setId(const QUaNodeId& id);

	// The Name Property is inherited from the TransitionVariableType.Its Value shall be the
	// BrowseName of one of the Transition Objects of the FiniteStateMachineType.
	
	// The Number Property is inherited from the TransitionVariableType.Its Value shall be the
	// TransitionNumber for one of the Transition Objects of the FiniteStateMachineType.

protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITETRANSITIONVARIABLE_H

