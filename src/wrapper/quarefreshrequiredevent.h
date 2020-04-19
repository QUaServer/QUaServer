#ifndef QUAREFRESHREQUIREDEVENT_H
#define QUAREFRESHREQUIREDEVENT_H

#include <QUaSystemEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

class QUaRefreshRequiredEvent : public QUaSystemEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaRefreshRequiredEvent(
		QUaServer *server
	);

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUAREFRESHREQUIREDEVENT_H