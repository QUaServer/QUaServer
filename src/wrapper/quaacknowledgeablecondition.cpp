#include "quaacknowledgeablecondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaAcknowledgeableCondition::QUaAcknowledgeableCondition(
	QUaServer *server
) : QUaCondition(server)
{
	
}



#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS