#include "quaexclusivelimitalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaExclusiveLimitStateMachine>

QUaExclusiveLimitAlarm::QUaExclusiveLimitAlarm(
	QUaServer* server
) : QUaLimitAlarm(server)
{
	
}

QUaExclusiveLimitStateMachine* QUaExclusiveLimitAlarm::getLimitState()
{
	return this->browseChild<QUaExclusiveLimitStateMachine>("LimitState");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS