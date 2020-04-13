#include "quafinitestatemachine.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteStateMachine::QUaFiniteStateMachine(
	QUaServer* server
) : QUaStateMachine(server)
{
	
}

QVector<QUaNodeId> QUaFiniteStateMachine::availableStates() const
{
	return const_cast<QUaFiniteStateMachine*>(this)->getAvailableStates()->value().value<QVector<QUaNodeId>>();
}

void QUaFiniteStateMachine::setAvailableStates(const QVector<QUaNodeId>& availableStates)
{
	this->getAvailableStates()->setValue(availableStates);
}

QVector<QUaNodeId> QUaFiniteStateMachine::availableTransitions() const
{
	return const_cast<QUaFiniteStateMachine*>(this)->getAvailableTransitions()->value().value<QVector<QUaNodeId>>();
}

void QUaFiniteStateMachine::setAvailableTransitions(const QVector<QUaNodeId>& availableTransitions)
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