#include "qopcuaabstractvariable.h"

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
	UA_Server_readValue(m_qopcuaserver->m_server, m_nodeId, &outValue);
	return QOpcUaTypesConverter::uaVariantToQVariant(outValue);
}

void QOpcUaAbstractVariable::set_value(const QVariant & value)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_Variant and set value
	UA_Server_writeValue(m_qopcuaserver->m_server, m_nodeId, QOpcUaTypesConverter::uaVariantFromQVariant(value));
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
	UA_Server_readDataType(m_qopcuaserver->m_server, m_nodeId, &outDataType);
	return QOpcUaTypesConverter::uaTypeNodeIdToQType(&outDataType);
}

void QOpcUaAbstractVariable::set_dataType(const QMetaType::Type & dataType)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_NodeId and set type
	UA_Server_writeDataType(m_qopcuaserver->m_server, m_nodeId, QOpcUaTypesConverter::uaTypeNodeIdFromQType(dataType));
}

qint32 QOpcUaAbstractVariable::get_valueRank() const
{
	return qint32();
}

void QOpcUaAbstractVariable::set_valueRank(const qint32 & valueRank)
{

}

quint32 QOpcUaAbstractVariable::get_arrayDimensionsSize() const
{
	return quint32();
}

void QOpcUaAbstractVariable::set_arrayDimensionsSize(const quint32 & arrayDimensionsSize)
{

}

QList<quint32> QOpcUaAbstractVariable::get_arrayDimensions() const
{
	return QList<quint32>();
}

void QOpcUaAbstractVariable::set_arrayDimensions(const QList<quint32>& arrayDimensions)
{

}

quint8 QOpcUaAbstractVariable::get_accessLevel() const
{
	return quint8();
}

void QOpcUaAbstractVariable::set_accessLevel(const quint8 & accessLevel)
{

}

double QOpcUaAbstractVariable::get_minimumSamplingInterval() const
{
	return 0.0;
}

void QOpcUaAbstractVariable::set_minimumSamplingInterval(const double & minimumSamplingInterval)
{

}

bool QOpcUaAbstractVariable::get_historizing() const
{
	return false;
}
