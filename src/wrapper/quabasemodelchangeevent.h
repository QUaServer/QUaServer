#ifndef QUABASEMODELCHANGEEVENT_H
#define QUABASEMODELCHANGEEVENT_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

class QUaBaseModelChangeEvent : public QUaBaseEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaBaseModelChangeEvent(
		QUaServer *server
	);

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUABASEMODELCHANGEEVENT_H