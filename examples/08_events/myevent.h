#ifndef MYEVENT_H
#define MYEVENT_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

class MyEvent : public QUaBaseEvent
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit MyEvent(QUaServer *server);

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // MYEVENT_H

