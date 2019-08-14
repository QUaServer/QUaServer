#include "quageneralmodelchangeevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

// Copy from parent and append 
const QStringList QUaGeneralModelChangeEvent::DefaultProperties = QStringList(QUaBaseEvent::DefaultProperties) << "Changes";

QUaGeneralModelChangeEvent::QUaGeneralModelChangeEvent(QUaServer *server)
	: QUaBaseEvent(server)
{
	
}

QUaProperty * QUaGeneralModelChangeEvent::getChanges()
{
	return this->findChild<QUaProperty*>("Changes");
}



#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS