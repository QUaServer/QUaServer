#include "qopcuabasevariable.h"

#include <QOpcUaServer>
#include <QOpcUaBaseDataVariable>

QOpcUaBaseVariable::QOpcUaBaseVariable(QOpcUaServerNode *parent) : QOpcUaServerNode(parent)
{

}

QVariant QOpcUaBaseVariable::get_value() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QVariant();
	}
	// read value
	UA_Variant outValue;
	auto st = UA_Server_readValue(m_qopcuaserver->m_server, m_nodeId, &outValue);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return QOpcUaTypesConverter::uaVariantToQVariant(outValue);
}

void QOpcUaBaseVariable::set_value(const QVariant & value)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// got modifiable copy
	auto newValue = value;
	// get types
	auto oldType = this->get_dataType();
	auto newType = (QMetaType::Type)newValue.type();
	// if variant list we need to adjust newType
	if (newValue.canConvert<QVariantList>())
	{
		auto iter = newValue.value<QSequentialIterable>();
		auto type = (QMetaType::Type)iter.at(0).type();
		if (iter.size() > 0)
		{
			QVariantList listOldType;
			newType = (QMetaType::Type)iter.at(0).type();
			// check uniform type throughout array
			for (int i = 0; i < iter.size(); i++)
			{
				auto tmpType = (QMetaType::Type)iter.at(i).type();
				Q_ASSERT_X(type == tmpType, "QOpcUaBaseVariable::set_value", "QVariant arrays must have same types in all its items.");
				if (type != tmpType)
				{
					return;
				}
				// preserve dataType if possible
				if (newType != oldType &&
					iter.at(0).canConvert(oldType))
				{
					listOldType.append(iter.at(i));
					listOldType[i].convert(oldType);
				}
			}
			// preserve dataType if possible
			if (newType != oldType &&
				iter.at(0).canConvert(oldType))
			{
				// overwrite newValue with converted list of old type
				newValue = listOldType;
				newType = oldType;
			}
		}
		else
		{
			newType = oldType;
		}
	}
	else if (newValue.canConvert(oldType)) // scalar
	{
		// preserve dataType if possible
		newValue.convert(oldType);
		newType = oldType;
	}
	// if new value dataType does not match the old value dataType
	// first set type to UA_NS0ID_BASEDATATYPE to avoid "BadTypeMismatch"	
	if (qobject_cast<QOpcUaBaseDataVariable*>(this) &&
		newType != oldType &&
		!newValue.canConvert(oldType))
	{
		auto st = UA_Server_writeDataType(m_qopcuaserver->m_server,
			m_nodeId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE));
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}

	// convert to UA_Variant and set new value
	auto st = UA_Server_writeValue(m_qopcuaserver->m_server,
		m_nodeId,
		QOpcUaTypesConverter::uaVariantFromQVariant(newValue));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit value changed
	emit this->valueChanged(newValue);
	// assign dataType, valueRank and arrayDimensions according to new value
	if (qobject_cast<QOpcUaBaseDataVariable*>(this))
	{
		// set new dataType if necessary
		if (newType != oldType &&
			!newValue.canConvert(oldType))
		{
			st = UA_Server_writeDataType(m_qopcuaserver->m_server,
				m_nodeId,
				QOpcUaTypesConverter::uaTypeNodeIdFromQType(newType));
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
			// emit dataType changed
			emit this->dataTypeChanged(newType);
		}
		// set valueRank and arrayDimensions if necessary
		if (newValue.canConvert<QVariantList>())
		{
			// set valueRank
			st = UA_Server_writeValueRank(m_qopcuaserver->m_server, m_nodeId, UA_VALUERANK_ONE_DIMENSION);
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
			// emit valueRank changed
			emit this->valueRankChanged(UA_VALUERANK_ONE_DIMENSION);
			// set arrayDimensions (bug : https://github.com/open62541/open62541/issues/2455)
			auto iter = newValue.value<QSequentialIterable>();
			auto size = (quint32)iter.size();
			// TODO : support multidimensional array (create custom Qt type compatible with QVariant)
			//        (UA_UInt32 *)(UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]));
			// https://www.bogotobogo.com/Qt/Qt5_QVariant_meta_object_system_MetaType.php
			UA_Variant uaArrayDimensions;
			UA_UInt32 arrayDims[1] = { size };
			UA_Variant_setArray(&uaArrayDimensions, arrayDims, 1, &UA_TYPES[UA_TYPES_UINT32]);
			UA_Server_writeArrayDimensions(m_qopcuaserver->m_server, m_nodeId, uaArrayDimensions);
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
			// emit arrayDimensions changed
			emit this->arrayDimensionsChanged(QVector<quint32>() << size);
		}
		// TODO : what happens with rank and arrayDim after going from array to scalar value?
	}
}

QMetaType::Type QOpcUaBaseVariable::get_dataType() const
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

void QOpcUaBaseVariable::set_dataType(const QMetaType::Type & dataType)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// need to "reset" dataType before setting a new value
	auto st = UA_Server_writeDataType(m_qopcuaserver->m_server,
		m_nodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// get old value
	QVariant oldValue = this->get_value();
	// handle array
	if (oldValue.canConvert<QVariantList>())
	{
		QVariantList listConvValues;
		auto iter = oldValue.value<QSequentialIterable>();
		// loop scalar or array
		for (int i = 0; i < iter.size(); i++)
		{
			QVariant varCurr = iter.at(i);
			if (varCurr.canConvert(dataType))
			{
				// convert in place
				varCurr.convert(dataType);
			}
			else
			{
				// else set default value for type
				varCurr = QVariant((QVariant::Type)dataType);
			}
			// append to list of converted values
			listConvValues.append(varCurr);
		}
		// overwrite old value
		oldValue = listConvValues;
	}
	// handle scalar
	else if (oldValue.canConvert(dataType))
	{
		// convert in place
		oldValue.convert(dataType);
	}
	else
	{
		// else set default value for type
		oldValue = QVariant((QVariant::Type)dataType);
	}

	// set converted or default value
	st = UA_Server_writeValue(m_qopcuaserver->m_server,
		m_nodeId,
		QOpcUaTypesConverter::uaVariantFromQVariant(oldValue));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// set new type
	st = UA_Server_writeDataType(m_qopcuaserver->m_server,
		m_nodeId,
		QOpcUaTypesConverter::uaTypeNodeIdFromQType(dataType));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit dataType changed
	emit this->dataTypeChanged(dataType);
	// emit value changed
	emit this->valueChanged(oldValue);
}

qint32 QOpcUaBaseVariable::get_valueRank() const
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

void QOpcUaBaseVariable::set_valueRank(const qint32 & valueRank)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));

	// TODO : fix value accordingly

	// set valueRank
	auto st = UA_Server_writeValueRank(m_qopcuaserver->m_server, m_nodeId, valueRank);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit valueRank changed
	emit this->valueRankChanged(UA_VALUERANK_ONE_DIMENSION);
}

QVector<quint32> QOpcUaBaseVariable::get_arrayDimensions() const
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
	Q_ASSERT(outArrayDimensions.type == &UA_TYPES[UA_TYPES_UINT32]);
	auto data = (quint32*)outArrayDimensions.data;
	for (int i = 0; i < outArrayDimensions.arrayLength; i++)
	{
		retArr.append(data[i]);
	}
	return retArr;
}

void QOpcUaBaseVariable::set_arrayDimensions(QVector<quint32>& arrayDimensions)
{
	// TODO : support multidimensional array
	//        (UA_UInt32 *)(UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]));
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert QList<quint32> to UA_Variant
	UA_Variant uaArrayDimensions;
	UA_Variant_setArray(&uaArrayDimensions,
		arrayDimensions.data(),
		arrayDimensions.count(),
		&UA_TYPES[UA_TYPES_UINT32]);

	// TODO : fix value accordingly

	// set arrayDimensions
	auto st = UA_Server_writeArrayDimensions(m_qopcuaserver->m_server, m_nodeId, uaArrayDimensions);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit arrayDimensions changed
	emit this->arrayDimensionsChanged(arrayDimensions);
}

quint8 QOpcUaBaseVariable::get_accessLevel() const
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

void QOpcUaBaseVariable::set_accessLevel(const quint8 & accessLevel)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set accessLevel
	auto st = UA_Server_writeAccessLevel(m_qopcuaserver->m_server, m_nodeId, accessLevel);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	emit this->accessLevelChanged(accessLevel);
}

double QOpcUaBaseVariable::get_minimumSamplingInterval() const
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
	// return
	return outMinimumSamplingInterval;
}

void QOpcUaBaseVariable::set_minimumSamplingInterval(const double & minimumSamplingInterval)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set minimumSamplingInterval
	auto st = UA_Server_writeMinimumSamplingInterval(m_qopcuaserver->m_server, m_nodeId, minimumSamplingInterval);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

bool QOpcUaBaseVariable::get_historizing() const
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

// [STATIC]
qint32 QOpcUaBaseVariable::GetValueRankFromQVariant(const QVariant & varValue)
{
	if ((QMetaType::Type)varValue.type() == QMetaType::UnknownType)
	{
		return UA_VALUERANK_ANY;
	}
	else if (varValue.canConvert<QVariantList>())
	{
		return UA_VALUERANK_ONE_DIMENSION;
	}
	// scalar is default
	return UA_VALUERANK_SCALAR;
}

// [STATIC]
QVector<quint32> QOpcUaBaseVariable::GetArrayDimensionsFromQVariant(const QVariant & varValue)
{
	if (varValue.canConvert<QVariantList>())
	{
		auto iter = varValue.value<QSequentialIterable>();
		auto size = (quint32)iter.size();
		return QVector<quint32>() << size;
	}
	// default arrayDimensionsSize == 0
	return QVector<quint32>();
}
