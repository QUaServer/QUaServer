#include "quaexclusivelimitalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaExclusiveLimitAlarm::QUaExclusiveLimitAlarm(
	QUaServer* server
) : QUaLimitAlarm(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS