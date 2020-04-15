#ifndef QUAREFRESHSTARTEVENT_H
#define QUAREFRESHSTARTEVENT_H

#include <QUaSystemEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

class QUaRefreshStartEvent : public QUaSystemEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaRefreshStartEvent(
		QUaServer *server
	);

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUAREFRESHSTARTEVENT_H