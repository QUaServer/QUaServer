#include "quaacknowledgeablecondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaAcknowledgeableCondition::QUaAcknowledgeableCondition(
	QUaServer *server
) : QUaCondition(server)
{
	
}

void QUaAcknowledgeableCondition::Acknowledge(QByteArray EventId, QString Comment)
{
	qDebug() << "Called Acknowledge on" << this->browseName()
		<< "EventId" << EventId << "Comment" << Comment;
}



#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS