#ifndef MYEVENTTYPE_H
#define MYEVENTTYPE_H

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaBaseEvent>

class MyEventType : public QUaBaseEvent
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit MyEventType(QUaServer *server);


	
};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // MYEVENTTYPE_H

