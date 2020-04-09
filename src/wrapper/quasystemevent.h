#ifndef QUASYSTEMEVENT_H
#define QUASYSTEMEVENT_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part 5 - 6.4.28

Concept introduced in Part 3 - 9.3
SystemEvents are Events of SystemEventType that are generated as a result of some Event
that occurs within the Server or by a system that the Server is representing.

This EventType inherits all Properties of the BaseEventType. Their semantic is defined in Part 5 - 6.4.2.
There are no additional Properties defined for this EventType.

*/

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