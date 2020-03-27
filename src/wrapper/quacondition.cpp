#include "quacondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaCondition::QUaCondition(
	QUaServer *server
) : QUaBaseEvent(server)
{
	
}

void QUaCondition::Enable()
{
	qDebug() << "Called Enable on" << this->browseName();
}

void QUaCondition::Disable()
{
	qDebug() << "Called Disable on" << this->browseName();
}

void QUaCondition::AddComment(QByteArray EventId, QString Comment)
{
	qDebug() << "Called AddComment on" << this->browseName()
		<< "EventId" << EventId << "Comment" << Comment;
}



#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS