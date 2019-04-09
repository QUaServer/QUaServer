#include "myeventtype.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

MyEventType::MyEventType(QUaServer *server)
	: QUaBaseEvent(server)
{

}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS