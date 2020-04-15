#ifndef QUAFINITESTATEVARIABLE_H
#define QUAFINITESTATEVARIABLE_H

#include <QUaStateVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaFiniteStateVariable : public QUaStateVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteStateVariable(
		QUaServer* server
	);

	// inherited

	// The NodeId of one of the state objects of the FiniteStateMachineType
	QUaNodeId id() const;
	void setId(const QUaNodeId&nodeId);

	// The Name is the BrowseName of one of the state objects of the FiniteStateMachineType

	// The Number is the state number of one of the state objects of the FiniteStateMachineType

protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITESTATEVARIABLE_H

