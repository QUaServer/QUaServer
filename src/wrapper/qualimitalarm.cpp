#include "qualimitalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaLimitAlarm::QUaLimitAlarm(
	QUaServer* server
) : QUaAlarmCondition(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS