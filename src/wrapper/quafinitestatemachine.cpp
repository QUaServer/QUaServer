#include "quafinitestatemachine.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteStateMachine::QUaFiniteStateMachine(
	QUaServer* server
) : QUaStateMachine(server)
{
	
}

QList<QUaNodeId> QUaFiniteStateMachine::availableStates() const
{
	return const_cast<QUaFiniteStateMachine*>(this)->getAvailableStates()->value<QList<QUaNodeId>>();
}

void QUaFiniteStateMachine::setAvailableStates(const QList<QUaNodeId>& availableStates)
{
	this->getAvailableStates()->setValue(availableStates);
}

QList<QUaNodeId> QUaFiniteStateMachine::availableTransitions() const
{
	return const_cast<QUaFiniteStateMachine*>(this)->getAvailableTransitions()->value<QList<QUaNodeId>>();
}

void QUaFiniteStateMachine::setAvailableTransitions(const QList<QUaNodeId>& availableTransitions)
{
	this->getAvailableTransitions()->setValue(availableTransitions);
}

QUaBaseDataVariable* QUaFiniteStateMachine::getAvailableStates()
{
	return this->browseChild<QUaBaseDataVariable>("AvailableStates", true);
}

QUaBaseDataVariable* QUaFiniteStateMachine::getAvailableTransitions()
{
	return this->browseChild<QUaBaseDataVariable>("AvailableTransitions", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS