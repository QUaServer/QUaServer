#include "quarefreshstartevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaRefreshStartEvent::QUaRefreshStartEvent(
	QUaServer *server
) : QUaSystemEvent(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS