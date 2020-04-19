#include "quarefreshrequiredevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaRefreshRequiredEvent::QUaRefreshRequiredEvent(
	QUaServer *server
) : QUaSystemEvent(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS