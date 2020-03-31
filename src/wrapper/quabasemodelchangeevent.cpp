#include "quabasemodelchangeevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaBaseModelChangeEvent::QUaBaseModelChangeEvent(
	QUaServer *server
) : QUaBaseEvent(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS