#ifndef QUAREFRESHSTARTEVENT_H
#define QUAREFRESHSTARTEVENT_H

#include <QUaSystemEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part 9 - 5.11.2

Used for Condition state synchronization (Part 9 - 4.5) in the 
ConditionRefresh Method (Part 9 - 5.5.7) and 
ConditionRefresh2 Method (Part 9 - 5.5.8)
See QUaCondition::ConditionRefresh and QUaCondition::ConditionRefresh2

*/

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