#include "quadiscretealarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaDiscreteAlarm::QUaDiscreteAlarm(
	QUaServer* server
) : QUaAlarmCondition(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS