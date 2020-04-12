#ifndef QUAFINITESTATEVARIABLE_H
#define QUAFINITESTATEVARIABLE_H

#include <QUaStateVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.6
/*
The FiniteStateVariableType is a subtype of StateVariableType and is used to store the current
state of a FiniteStateMachine as a human readable name.

*/

class QUaFiniteStateVariable : public QUaStateVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteStateVariable(
		QUaServer* server
	);

	// inherited

	// Id is inherited from the StateVariableType and overridden to reflect the required DataType. 
	// This value shall be the NodeId of one of the State Objects of the FiniteStateMachineType
	QString id() const;
	void setId(const QString &nodeId);

	// The Name Property is inherited from StateVariableType.Its Value shall be the BrowseName of
	// one of the State Objects of the FiniteStateMachineType.

	// The Number Property is inherited from StateVariableType.Its Value shall be the StateNumber
	// for one of the State Objects of the FiniteStateMachineType.

protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITESTATEVARIABLE_H

