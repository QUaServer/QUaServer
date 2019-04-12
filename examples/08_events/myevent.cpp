#include "myevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

MyEvent::MyEvent(QUaServer *server)
	: QUaBaseEvent(server)
{

}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS