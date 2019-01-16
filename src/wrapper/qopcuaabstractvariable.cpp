#include "qopcuaabstractvariable.h"

#include <QOpcUaBaseDataVariable>

QOpcUaAbstractVariable::QOpcUaAbstractVariable(QOpcUaServerNode *parent) : QOpcUaServerNode(parent)
{

}

QVariant QOpcUaAbstractVariable::get_value() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read value
	UA_Variant outValue;
	auto st = UA_Server_readValue(m_qopcuaserver->m_server, m_nodeId, &outValue);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return QOpcUaTypesConverter::uaVariantToQVariant(outValue);
}

void QOpcUaAbstractVariable::set_value(const QVariant & value)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_Variant and set value
	auto st = UA_Server_writeValue(m_qopcuaserver->m_server, m_nodeId, QOpcUaTypesConverter::uaVariantFromQVariant(value));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// if instance of QOpcUaBaseDataVariable, then also assign dataType, valueRank and arrayDimensions
	if (qobject_cast<QOpcUaBaseDataVariable*>(this))
	{
		this->set_dataType((QMetaType::Type)value.type());
		// TODO : valueRank and arrayDimensions
	}
}

QMetaType::Type QOpcUaAbstractVariable::get_dataType() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QMetaType::UnknownType;
	}
	// read type
	UA_NodeId outDataType;
	auto st = UA_Server_readDataType(m_qopcuaserver->m_server, m_nodeId, &outDataType);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return QOpcUaTypesConverter::uaTypeNodeIdToQType(&outDataType);
}

void QOpcUaAbstractVariable::set_dataType(const QMetaType::Type & dataType)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_NodeId and set type
	auto st = UA_Server_writeDataType(m_qopcuaserver->m_server, m_nodeId, QOpcUaTypesConverter::uaTypeNodeIdFromQType(dataType));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

qint32 QOpcUaAbstractVariable::get_valueRank() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return -1;
	}
	// read valueRank
	qint32 outValueRank;
	auto st = UA_Server_readValueRank(m_qopcuaserver->m_server, m_nodeId, &outValueRank);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outValueRank;
}

void QOpcUaAbstractVariable::set_valueRank(const qint32 & valueRank)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set valueRank
	UA_Server_writeValueRank(m_qopcuaserver->m_server, m_nodeId, valueRank);
}

QVector<quint32> QOpcUaAbstractVariable::get_arrayDimensions() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QVector<quint32>();
	}
	// read arrayDimensionsSize
	UA_Variant outArrayDimensions;
	auto st = UA_Server_readArrayDimensions(m_qopcuaserver->m_server, m_nodeId, &outArrayDimensions);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// convert UA_Variant to QList<quint32>
	QVector<quint32> retArr;
	for (int i = 0; i < outArrayDimensions.arrayDimensionsSize; i++)
	{
		retArr.append(outArrayDimensions.arrayDimensions[i]);
	}
	return retArr;
}

void QOpcUaAbstractVariable::set_arrayDimensions(QVector<quint32>& arrayDimensions)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert QList<quint32> to UA_Variant
	UA_Variant uaArrayDimensions;
	uaArrayDimensions.arrayDimensionsSize = arrayDimensions.count();
	uaArrayDimensions.arrayDimensions     = arrayDimensions.data();
	// set arrayDimensions
	auto st = UA_Server_writeArrayDimensions(m_qopcuaserver->m_server, m_nodeId, uaArrayDimensions);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

quint8 QOpcUaAbstractVariable::get_accessLevel() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return 0;
	}
	// read accessLevel
	UA_Byte outAccessLevel;
	auto st = UA_Server_readAccessLevel(m_qopcuaserver->m_server, m_nodeId, &outAccessLevel);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outAccessLevel;
}

void QOpcUaAbstractVariable::set_accessLevel(const quint8 & accessLevel)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set accessLevel
	auto st = UA_Server_writeAccessLevel(m_qopcuaserver->m_server, m_nodeId, accessLevel);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

double QOpcUaAbstractVariable::get_minimumSamplingInterval() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return 0.0;
	}
	// read minimumSamplingInterval
	UA_Double outMinimumSamplingInterval;
	auto st = UA_Server_readMinimumSamplingInterval(m_qopcuaserver->m_server, m_nodeId, &outMinimumSamplingInterval);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

void QOpcUaAbstractVariable::set_minimumSamplingInterval(const double & minimumSamplingInterval)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set minimumSamplingInterval
	auto st = UA_Server_writeMinimumSamplingInterval(m_qopcuaserver->m_server, m_nodeId, minimumSamplingInterval);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

bool QOpcUaAbstractVariable::get_historizing() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return false;
	}
	// read historizing
	UA_Boolean outHistorizing;
	auto st = UA_Server_readHistorizing(m_qopcuaserver->m_server, m_nodeId, &outHistorizing);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outHistorizing;
}
