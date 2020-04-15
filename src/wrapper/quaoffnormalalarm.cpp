#include "quaoffnormalalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaOffNormalAlarm::QUaOffNormalAlarm(
	QUaServer* server
) : QUaDiscreteAlarm(server)
{
	
}

QUaNodeId QUaOffNormalAlarm::normalState() const
{
	return const_cast<QUaOffNormalAlarm*>(this)->getNormalState()->value<QUaNodeId>();
}

void QUaOffNormalAlarm::setNormalState(const QUaNodeId& normalState)
{
	this->getNormalState()->setValue(normalState);
}

QUaProperty* QUaOffNormalAlarm::getNormalState()
{
	return this->browseChild<QUaProperty>("NormalState");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS