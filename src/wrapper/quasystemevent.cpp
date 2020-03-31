#include "quasystemevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaSystemEvent::QUaSystemEvent(
	QUaServer *server
) : QUaBaseEvent(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS