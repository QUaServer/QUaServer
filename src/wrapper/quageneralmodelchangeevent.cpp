#include "quageneralmodelchangeevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

// Copy from parent and append 
const QStringList QUaGeneralModelChangeEvent::DefaultProperties = QStringList(QUaBaseEvent::DefaultProperties) << "Changes";

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
	/*
	this->getChanges()->setArrayDimensions(listVerbs.count());
	*/
	/*
	// TEST
	auto dims = this->getChanges()->arrayDimensions();
	Q_ASSERT(dims[0] == listVerbs.count());
	auto rank = this->getChanges()->valueRank();
	Q_ASSERT(rank == UA_VALUERANK_ONE_DIMENSION);
	auto dataType = this->getChanges()->dataType();
	Q_ASSERT(dataType == METATYPE_CHANGESTRUCTUREDATATYPE);
	qDebug() << "Changes NodeId" << this->getChanges()->nodeId();
	qDebug() << "Changes DataType NodeId" << this->getChanges()->dataTypeNodeId();
	qDebug() << "Changes TypeDef NodeId"  << this->getChanges()->typeDefinitionNodeId();
	*/
}

QUaProperty * QUaGeneralModelChangeEvent::getChanges() const
{
	return this->findChild<QUaProperty*>("Changes");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS