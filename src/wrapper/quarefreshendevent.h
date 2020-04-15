#ifndef QUAREFRESHENDEVENT_H
#define QUAREFRESHENDEVENT_H

#include <QUaSystemEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

class QUaRefreshEndEvent : public QUaSystemEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaRefreshEndEvent(
		QUaServer *server
	);

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUAREFRESHENDEVENT_H