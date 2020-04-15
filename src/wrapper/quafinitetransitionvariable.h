#ifndef QUAFINITETRANSITIONVARIABLE_H
#define QUAFINITETRANSITIONVARIABLE_H

#include <QUaTransitionVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaFiniteTransitionVariable : public QUaTransitionVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteTransitionVariable(
		QUaServer* server
	);

	// inherited

	// The NodeId of one of the transition objects of the FiniteStateMachineType
	QUaNodeId id() const;
	void setId(const QUaNodeId& id);

	// The Name is the BrowseName of one of the transition objects of the FiniteStateMachineType
	
	// The Number is the TransitionNumber of one of the transition objects of the FiniteStateMachineType

protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITETRANSITIONVARIABLE_H

