#ifndef QUACONDITION_H
#define QUACONDITION_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part X - X.X ConditionType


*/


class QUaCondition : public QUaBaseEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaCondition(
		QUaServer *server
	);



private:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUACONDITION_H