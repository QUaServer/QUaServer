#ifndef QUASYSTEMEVENT_H
#define QUASYSTEMEVENT_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

class QUaSystemEvent : public QUaBaseEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaSystemEvent(
		QUaServer *server
	);

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUASYSTEMEVENT_H