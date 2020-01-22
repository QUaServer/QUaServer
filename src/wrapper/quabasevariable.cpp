#include "quabasevariable.h"

#include <QUaServer>
#include <QUaBaseDataVariable>

QMetaEnum QUaDataType::m_metaEnum = QMetaEnum::fromType<QUa::Type>();

QUaDataType::QUaDataType()
{
	m_type = QUa::Type::UnknownType;
}

QUaDataType::QUaDataType(const QMetaType::Type& metaType)
{
	m_type = static_cast<QUa::Type>(metaType);
}

QUaDataType::QUaDataType(const QString& strType)
{
	*this = QUaDataType(strType.toUtf8());
}

QUaDataType::QUaDataType(const QByteArray& byteType)
{
	bool ok = false;
	int val = m_metaEnum.keyToValue(byteType.constData(), &ok);
	m_type = static_cast<QUa::Type>(val);
}

QUaDataType::operator QMetaType::Type()
{
	return static_cast<QMetaType::Type>(m_type);
}

QUaDataType::operator QString()
{
	return QString(m_metaEnum.valueToKey(static_cast<int>(m_type)));
}

bool QUaDataType::operator==(const QMetaType::Type& metaType)
{
	return static_cast<QMetaType::Type>(m_type) == metaType;
}

// [STATIC]
void QUaBaseVariable::onWrite(UA_Server             *server, 
		                      const UA_NodeId       *sessionId,
		                      void                  *sessionContext, 
		                      const UA_NodeId       *nodeId,
		                      void                  *nodeContext, 
		                      const UA_NumericRange *range,
		                      const UA_DataValue    *data)
{
	Q_UNUSED(server);
	Q_UNUSED(sessionId);
	Q_UNUSED(sessionContext);
	Q_UNUSED(nodeId);
	Q_UNUSED(range);
	Q_UNUSED(data);
	// get variable from context
#ifdef QT_DEBUG 
	auto var = dynamic_cast<QUaBaseVariable*>(static_cast<QObject*>(nodeContext));
	Q_CHECK_PTR(var);
#else
	auto var = static_cast<QUaBaseVariable*>(nodeContext);
#endif // QT_DEBUG 
	if (!var)
	{
		return;
	}
	// do not emit if value change is internal
	if (var->m_bInternalWrite)
	{
		var->m_bInternalWrite = false;
		return;
	}
	// emit value changed
	emit var->valueChanged(var->value());
}

// [STATIC]
void QUaBaseVariable::onRead(UA_Server             *server, 
		                      const UA_NodeId       *sessionId,
		                      void                  *sessionContext, 
		                      const UA_NodeId       *nodeId,
		                      void                  *nodeContext, 
		                      const UA_NumericRange *range,
		                      const UA_DataValue    *data)
{
	Q_UNUSED(server);
	Q_UNUSED(sessionId);
	Q_UNUSED(sessionContext);
	Q_UNUSED(nodeId);
	Q_UNUSED(range);
	Q_UNUSED(data);
	// get variable from context
#ifdef QT_DEBUG 
	auto var = dynamic_cast<QUaBaseVariable*>(static_cast<QObject*>(nodeContext));
	Q_CHECK_PTR(var);
#else
	auto var = static_cast<QUaBaseVariable*>(nodeContext);
#endif // QT_DEBUG 
	if (!var)
	{
		return;
	}
	if (!var->m_readCallback || var->m_readCallbackRunning) return;
	// setValue (somehow) triggers read callback again; this avoids recursion
	QVariant newValue = var->m_readCallback();
	if (!newValue.isNull())
	{
		var->m_readCallbackRunning = true;
		var->setValue(newValue);
		var->m_readCallbackRunning = false;
	}
}

QUaBaseVariable::QUaBaseVariable(QUaServer *server)
	: QUaNode(server)
{
	// [NOTE] : constructor of any QUaNode-derived class is not meant to be called by the user
	//          the constructor is called automagically by this library, and m_newNodeNodeId and
	//          m_newNodeMetaObject must be set in QUaServer before calling the constructor, as
	//          is used in QUaServer::uaConstructor
	Q_CHECK_PTR(server);
	Q_CHECK_PTR(server->m_newNodeNodeId);
	m_type = QMetaType::UnknownType;
	// this should not be needed since stored in m_nodeId already??:
	//    const UA_NodeId &nodeId = *server->m_newNodeNodeId;
	// sets also write callback to emit onWrite signal
	setReadCallback();
}

void QUaBaseVariable::setReadCallback(const std::function<QVariant()>& readCallback){
	UA_ValueCallback callback;
	if (readCallback)
	{
		callback.onRead = &QUaBaseVariable::onRead;
		m_readCallback = readCallback;
		m_readCallbackRunning = false;
	}
	else
	{
		callback.onRead = nullptr;
		m_readCallback = readCallback;
	}
	callback.onWrite = &QUaBaseVariable::onWrite;
	// this replaces the previous callback, if any
	UA_Server_setVariableNode_valueCallback(m_qUaServer->m_server, m_nodeId, callback);
}

QVariant QUaBaseVariable::value() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QVariant();
	}
	// read value
	UA_Variant outValue;
	auto st = UA_Server_readValue(m_qUaServer->m_server, m_nodeId, &outValue);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// convert
	QVariant outVar = QUaTypesConverter::uaVariantToQVariant(outValue);
	// clenaup
	UA_Variant_deleteMembers(&outValue);
	return outVar;
}

void QUaBaseVariable::setValue(const QVariant & value, QMetaType::Type newType/* = QMetaType::UnknownType*/)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (newType == QMetaType::UnknownType)
	{
		newType = (QMetaType::Type)value.type();
	}
	// got modifiable copy
	auto newValue = value;
	// get types
	auto oldType = this->dataType();
	// if variant list we need to adjust newType
	if (newValue.canConvert<QVariantList>())
	{
		auto iter = newValue.value<QSequentialIterable>();
		// TODO : [BUG] what happens if iter size is zero !?
		auto type = (QMetaType::Type)iter.at(0).type();
		if (iter.size() > 0)
		{
			QVariantList listOldType;
			if (newType == QMetaType::UnknownType || newType == QMetaType::User)
			{
				newType = (QMetaType::Type)iter.at(0).type();
			}
			// check uniform type throughout array
			for (int i = 0; i < iter.size(); i++)
			{
				auto tmpType = (QMetaType::Type)iter.at(i).type();
				Q_ASSERT_X(type == tmpType, "QUaBaseVariable::setValue", "QVariant arrays must have same types in all its items.");
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
	if (newType != oldType &&
		!newValue.canConvert(oldType))
	{
		auto st = UA_Server_writeDataType(m_qUaServer->m_server,
			m_nodeId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE));
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	// convert to UA_Variant and set new value
	auto tmpVar = QUaTypesConverter::uaVariantFromQVariant(newValue, newType);
	m_bInternalWrite = true;
	auto st = UA_Server_writeValue(m_qUaServer->m_server,
		m_nodeId,
		tmpVar);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_Variant_clear(&tmpVar);
	// set new dataType if necessary
	if (newType != oldType &&
		!newValue.canConvert(oldType))
	{
		st = UA_Server_writeDataType(m_qUaServer->m_server,
			m_nodeId,
			QUaTypesConverter::uaTypeNodeIdFromQType(newType));
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	// [NOTE] do not set rank or arrayDimensions because they are permanent
	//        is better to just set array dimensions on Variant value and leave rank as ANY
	// update cache
	m_type = newType;
	//Q_ASSERT(this->dataTypeInternal() == m_type);
}

QMetaType::Type QUaBaseVariable::dataType() const
{
	return m_type;
}

QString QUaBaseVariable::dataTypeNodeId() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return  QUaTypesConverter::nodeIdToQString(UA_NODEID_NULL);
	}
	// read type
	UA_NodeId outDataType;
	auto st = UA_Server_readDataType(m_qUaServer->m_server, m_nodeId, &outDataType);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// check if type is enum, if so, return type int 32
	if (!m_qUaServer->m_hashEnums.key(outDataType, "").isEmpty())
	{
		UA_NodeId_clear(&outDataType);
		return QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_INT32));
	}
	// else return converted type
	QString retNodeId = QUaTypesConverter::nodeIdToQString(outDataType);
	UA_NodeId_clear(&outDataType);
	return retNodeId;
}

void QUaBaseVariable::setDataType(const QMetaType::Type & dataType)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// need to "reset" dataType before setting a new value
	auto st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// get old value
	QVariant oldValue = this->value();
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
	auto tmpVar = QUaTypesConverter::uaVariantFromQVariant(oldValue, dataType);
	m_bInternalWrite = true;
	st = UA_Server_writeValue(m_qUaServer->m_server,
		m_nodeId,
		tmpVar);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_Variant_clear(&tmpVar);
	// set new type
	st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		QUaTypesConverter::uaTypeNodeIdFromQType(dataType));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// update cache
	m_type = dataType;
	Q_ASSERT(this->dataTypeInternal() == m_type);
}

void QUaBaseVariable::setDataTypeEnum(const QMetaEnum & metaEnum)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// compose enum name
	#if QT_VERSION >= 0x051200
		QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
	#else
		QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.name());
	#endif
	// register if not exists
	if (!m_qUaServer->m_hashEnums.contains(strEnumName))
	{
		m_qUaServer->registerEnum(metaEnum);
	}
	Q_ASSERT(m_qUaServer->m_hashEnums.contains(strEnumName));
	// get enum nodeId
	UA_NodeId enumTypeNodeId = m_qUaServer->m_hashEnums.value(strEnumName);
	// call internal method
	this->setDataTypeEnum(enumTypeNodeId);
}

bool QUaBaseVariable::setDataTypeEnum(const QString & strEnumName)
{
	// check if exists in server's hash
	if (!m_qUaServer->m_hashEnums.contains(strEnumName))
	{
		return false;
	}
	// get enum nodeId
	UA_NodeId enumTypeNodeId = m_qUaServer->m_hashEnums.value(strEnumName);
	// call internal method
	this->setDataTypeEnum(enumTypeNodeId);
	// success
	return true;
}

void QUaBaseVariable::setDataTypeEnum(const UA_NodeId & enumTypeNodeId)
{
	// need to "reset" dataType before setting a new value
	auto st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// get old value
	QVariant oldValue = this->value();
	// handle array
	if (oldValue.canConvert<QVariantList>())
	{
		QVariantList listConvValues;
		auto iter = oldValue.value<QSequentialIterable>();
		// get first value if any
		QVariant varFirst = iter.size() > 0 ? iter.at(0) : QVariant((QVariant::Type)QMetaType::Int);
		// overwrite old value
		oldValue = varFirst;
	}
	// handle scalar
	if (oldValue.canConvert(QMetaType::Int))
	{
		// convert in place
		oldValue.convert(QMetaType::Int);
	}
	else
	{
		// else set default value for type
		oldValue = QVariant((QVariant::Type)QMetaType::Int);
	}
	// set converted or default value
	auto tmpVar = QUaTypesConverter::uaVariantFromQVariant(oldValue);
	m_bInternalWrite = true;
	st = UA_Server_writeValue(m_qUaServer->m_server,
		m_nodeId,
		tmpVar);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_Variant_clear(&tmpVar);
	// change data type
	st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		enumTypeNodeId);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// update cache
	m_type = QMetaType::Int;
	Q_ASSERT(this->dataTypeInternal() == m_type);
}

QMetaType::Type QUaBaseVariable::dataTypeInternal() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QMetaType::UnknownType;
	}
	// read type
	UA_NodeId outDataType;
	auto st = UA_Server_readDataType(m_qUaServer->m_server, m_nodeId, &outDataType);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// check if type is enum, if so, return type int 32
	if (!m_qUaServer->m_hashEnums.key(outDataType, "").isEmpty())
	{
		UA_NodeId_clear(&outDataType);
		return QMetaType::Int;
	}
	// else return converted type
	QMetaType::Type type = QUaTypesConverter::uaTypeNodeIdToQType(&outDataType);
	UA_NodeId_clear(&outDataType);
	return type;
}

qint32 QUaBaseVariable::valueRank() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return -1;
	}
	// read valueRank
	qint32 outValueRank;
	auto st = UA_Server_readValueRank(m_qUaServer->m_server, m_nodeId, &outValueRank);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outValueRank;
}

void QUaBaseVariable::setValueRank(const qint32& valueRank){
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	auto st = UA_Server_writeValueRank(m_qUaServer->m_server, m_nodeId, valueRank);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

QVector<quint32> QUaBaseVariable::arrayDimensions() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QVector<quint32>();
	}
	// read arrayDimensionsSize
	UA_Variant outArrayDimensions;
	auto st = UA_Server_readArrayDimensions(m_qUaServer->m_server, m_nodeId, &outArrayDimensions);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// convert UA_Variant to QList<quint32>
	QVector<quint32> retArr;
	Q_ASSERT(outArrayDimensions.type == &UA_TYPES[UA_TYPES_UINT32]);
	auto data = (quint32*)outArrayDimensions.data;
	for (int i = 0; i < (int)outArrayDimensions.arrayLength; i++)
	{
		retArr.append(data[i]);
	}
	return retArr;
}

/*
void QUaBaseVariable::setArrayDimensions(const quint32 &size) // const QVector<quint32> &arrayDimenstions
{
	UA_Variant uaArrayDimensions;
	UA_UInt32 arrayDims[1] = { size };
	UA_Variant_setArray(&uaArrayDimensions, arrayDims, 1, &UA_TYPES[UA_TYPES_UINT32]);
	UA_Server_writeArrayDimensions(m_qUaServer->m_server, m_nodeId, uaArrayDimensions);
}
*/

quint8 QUaBaseVariable::accessLevel() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return 0;
	}
	// read accessLevel
	UA_Byte outAccessLevel;
	auto st = UA_Server_readAccessLevel(m_qUaServer->m_server, m_nodeId, &outAccessLevel);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outAccessLevel;
}

void QUaBaseVariable::setAccessLevel(const quint8 & accessLevel)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set accessLevel
	auto st = UA_Server_writeAccessLevel(m_qUaServer->m_server, m_nodeId, accessLevel);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

double QUaBaseVariable::minimumSamplingInterval() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return 0.0;
	}
	// read minimumSamplingInterval
	UA_Double outMinimumSamplingInterval;
	auto st = UA_Server_readMinimumSamplingInterval(m_qUaServer->m_server, m_nodeId, &outMinimumSamplingInterval);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return
	return outMinimumSamplingInterval;
}

void QUaBaseVariable::setMinimumSamplingInterval(const double & minimumSamplingInterval)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set minimumSamplingInterval
	auto st = UA_Server_writeMinimumSamplingInterval(m_qUaServer->m_server, m_nodeId, minimumSamplingInterval);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

bool QUaBaseVariable::historizing() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return false;
	}
	// read historizing
	UA_Boolean outHistorizing;
	auto st = UA_Server_readHistorizing(m_qUaServer->m_server, m_nodeId, &outHistorizing);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outHistorizing;
}

bool QUaBaseVariable::readAccess() const
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue = this->accessLevel();
	return accessLevel.bits.bRead;
}

void QUaBaseVariable::setReadAccess(const bool & readAccess)
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue   = this->accessLevel();
	accessLevel.bits.bRead = readAccess;
	this->setAccessLevel(accessLevel.intValue);
}

bool QUaBaseVariable::writeAccess() const
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue = this->accessLevel();
	return accessLevel.bits.bWrite;
}

void QUaBaseVariable::setWriteAccess(const bool & writeAccess)
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue    = this->accessLevel();
	accessLevel.bits.bWrite = writeAccess;
	this->setAccessLevel(accessLevel.intValue);
}

// [STATIC]
qint32 QUaBaseVariable::GetValueRankFromQVariant(const QVariant & varValue)
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
QVector<quint32> QUaBaseVariable::GetArrayDimensionsFromQVariant(const QVariant & varValue)
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
