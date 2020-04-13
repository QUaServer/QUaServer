#include "quafinitetransitionvariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteTransitionVariable::QUaFiniteTransitionVariable(
	QUaServer* server
) : QUaTransitionVariable(server)
{
	
}

QUaNodeId QUaFiniteTransitionVariable::id() const
{
	return const_cast<QUaFiniteTransitionVariable*>(this)->getId()->value<QUaNodeId>();
}

void QUaFiniteTransitionVariable::setId(const QUaNodeId& nodeId)
{
	this->getId()->setValue(nodeId);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS