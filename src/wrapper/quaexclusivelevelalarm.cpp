#include "quaexclusivelevelalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaExclusiveLevelAlarm::QUaExclusiveLevelAlarm(
	QUaServer* server
) : QUaExclusiveLimitAlarm(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS