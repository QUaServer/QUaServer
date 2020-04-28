#include "quarefreshrequiredevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaRefreshRequiredEvent::QUaRefreshRequiredEvent(
	QUaServer *server
) : QUaSystemEvent(server)
{
#ifdef UA_ENABLE_HISTORIZING
	m_historizing = false;
#endif // UA_ENABLE_HISTORIZING
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS