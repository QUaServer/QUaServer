#include "quacondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaCondition::QUaCondition(
	QUaServer *server
) : QUaBaseEvent(server)
{
	
}



#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS