#include "quaoffnormalalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaOffNormalAlarm::QUaOffNormalAlarm(
	QUaServer* server
) : QUaDiscreteAlarm(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS