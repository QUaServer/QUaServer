#ifndef QUAACKNOWLEDGEABLECONDITION_H
#define QUAACKNOWLEDGEABLECONDITION_H

#include <QUaCondition>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part X - X.X ConditionType


*/


class QUaAcknowledgeableCondition : public QUaCondition
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaAcknowledgeableCondition(
		QUaServer *server
	);



private:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUAACKNOWLEDGEABLECONDITION_H