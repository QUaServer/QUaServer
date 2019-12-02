#include "quageneralmodelchangeevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

// Copy from parent and append new ones
const QStringList QUaGeneralModelChangeEvent::DefaultProperties = 
	QStringList(QUaBaseEvent::DefaultProperties) << "Changes";

QUaGeneralModelChangeEvent::QUaGeneralModelChangeEvent(QUaServer *server)
	: QUaBaseEvent(server)
{
	
}

QUaChangesList QUaGeneralModelChangeEvent::changes() const
{
	QUaChangesList retList;
	QVariant varList = this->getChanges()->value();
	if (!varList.isValid() || !varList.canConvert<QVariantList>())
	{
		return retList;
	}
	auto iter = varList.value<QSequentialIterable>();
	for (const QVariant &v : iter)
	{
		retList << v.value<QUaChangeStructureDataType>();
	}
	return retList;
}

void QUaGeneralModelChangeEvent::setChanges(const QUaChangesList & listVerbs)
{
	this->getChanges()->setValue(QVariant::fromValue(listVerbs), METATYPE_CHANGESTRUCTUREDATATYPE);
}

QUaProperty * QUaGeneralModelChangeEvent::getChanges() const
{
	return this->findChild<QUaProperty*>("Changes");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS