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

void QUaAcknowledgeableCondition::Confirm(QByteArray EventId, QString Comment)
{
	qDebug() << "Called Confirm on" << this->browseName()
		<< "EventId" << EventId << "Comment" << Comment;
}

bool QUaAcknowledgeableCondition::confirmAllowed() const
{
	return this->hasOptionalMethod("Confirm");
}

void QUaAcknowledgeableCondition::setConfirmAllowed(const bool& confirmAllowed)
{
	confirmAllowed ?
		this->addOptionalMethod("Confirm") :
		this->removeOptionalMethod("Confirm");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS