#include "quafinitestatevariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteStateVariable::QUaFiniteStateVariable(
	QUaServer* server
) : QUaStateVariable(server)
{
	
}

QString QUaFiniteStateVariable::id() const
{
	return const_cast<QUaFiniteStateVariable*>(this)->getId()->value().toString();
}

void QUaFiniteStateVariable::setId(const QString& nodeId)
{
	this->getId()->setValue(nodeId);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS