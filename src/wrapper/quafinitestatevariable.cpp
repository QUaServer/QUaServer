#include "quafinitestatevariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteStateVariable::QUaFiniteStateVariable(
	QUaServer* server
) : QUaStateVariable(server)
{
	
}

QUaNodeId QUaFiniteStateVariable::id() const
{
	return const_cast<QUaFiniteStateVariable*>(this)->getId()->value<QUaNodeId>();
}

void QUaFiniteStateVariable::setId(const QUaNodeId& nodeId)
{
	this->getId()->setValue(nodeId);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS