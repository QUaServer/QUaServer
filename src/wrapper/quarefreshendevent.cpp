#include "quarefreshendevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaRefreshEndEvent::QUaRefreshEndEvent(
	QUaServer *server
) : QUaSystemEvent(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS