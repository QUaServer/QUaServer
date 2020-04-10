#include "quatransition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaTransition::QUaTransition(
	QUaServer* server
) : QUaBaseObject(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS