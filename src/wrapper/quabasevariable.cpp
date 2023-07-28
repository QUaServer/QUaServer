#include "quabasevariable.h"

#include "quaserver_anex.h"
#include <QUaBaseDataVariable>

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
#ifndef OPEN62541_ISSUE3934_RESOLVED
// NOTE : added this ugly workaround that passes the "same datatypes" condition in
// server/ua_services_attribute.c::compatibleDataType so we can actually set the value
// of an optionset satatype
/* OptionSet */
static UA_DataTypeMember OptionSet_members[2] = {
{
#if UA_OPEN62541_VER_MINOR < 3
	UA_TYPES_BYTESTRING, /* .memberTypeIndex */
	0, /* .padding */
	true, /* .namespaceZero */
	false, /* .isArray */
	false  /* .isOptional */
	UA_TYPENAME("Value") /* .memberName */
},
{
	UA_TYPES_BYTESTRING, /* .memberTypeIndex */
	offsetof(UA_OptionSet, validBits) - offsetof(UA_OptionSet, value) - sizeof(UA_ByteString), /* .padding */
	true, /* .namespaceZero */
	false, /* .isArray */
	false  /* .isOptional */
	UA_TYPENAME("ValidBits") /* .memberName */
#else
	UA_TYPENAME("Value") /* .memberName */
	&UA_TYPES[UA_TYPES_BYTESTRING], /* .memberType */
	0, /* .padding */
	false, /* .isArray */
	false  /* .isOptional */
},
{
	UA_TYPENAME("ValidBits") /* .memberName */
	&UA_TYPES[UA_TYPES_BYTESTRING], /* .memberType */
	offsetof(UA_OptionSet, validBits) - offsetof(UA_OptionSet, value) - sizeof(UA_ByteString), /* .padding */
	false, /* .isArray */
	false  /* .isOptional */
#endif
}, };

QHash<UA_NodeId, UA_DataType*> mapOptionSetDatatypes;
UA_DataType* getDataTypeFromNodeId(const UA_NodeId& optNodeId)
{
	if (mapOptionSetDatatypes.contains(optNodeId))
	{
		return mapOptionSetDatatypes[optNodeId];
	}
	UA_DataType* tmpType = new UA_DataType({
#if UA_OPEN62541_VER_MINOR < 3
		optNodeId, /* .typeId */
		{0, UA_NODEIDTYPE_NUMERIC, {12765}}, /* .binaryEncodingId */
		sizeof(UA_OptionSet), /* .memSize */
		UA_TYPES_OPTIONSET, /* .typeIndex */
		UA_DATATYPEKIND_STRUCTURE, /* .typeKind */
		false, /* .pointerFree */
		false, /* .overlayable */
		2, /* .membersSize */
		OptionSet_members  /* .members */
		UA_TYPENAME("OptionSet") /* .typeName */
#else
		UA_TYPENAME("OptionSet") /* .typeName */
		optNodeId, /* .typeId */
		{0, UA_NODEIDTYPE_NUMERIC, {12765LU}}, /* .binaryEncodingId */
		sizeof(UA_OptionSet), /* .memSize */
		UA_DATATYPEKIND_STRUCTURE, /* .typeKind */
		false, /* .pointerFree */
		false, /* .overlayable */
		2, /* .membersSize */
		OptionSet_members  /* .members */
#endif
	});
	mapOptionSetDatatypes[optNodeId] = tmpType;
	return tmpType;
}
#endif // !OPEN62541_ISSUE3934_RESOLVED
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL

// [STATIC] : always called by open62541 library after a write, used by QUaServer 
// to emit signals and to make differentiation between network or programmatic value change
void QUaBaseVariable::onWrite(UA_Server             *server, 
		                      const UA_NodeId       *sessionId,
		                      void                  *sessionContext, 
		                      const UA_NodeId       *nodeId,
		                      void                  *nodeContext, 
		                      const UA_NumericRange *range,
		                      const UA_DataValue    *data)
{
	Q_UNUSED(sessionContext);
	Q_UNUSED(nodeId);
	Q_UNUSED(range);
	// get variable from context
#ifdef QT_DEBUG 
	auto var = qobject_cast<QUaBaseVariable*>(static_cast<QObject*>(nodeContext));
	Q_CHECK_PTR(var);
#else
	auto var = static_cast<QUaBaseVariable*>(nodeContext);
#endif // QT_DEBUG 
	if (!var)
	{
		return;
	}
	// get server
	void* serverContext = nullptr;
	auto st = UA_Server_getNodeContext(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), &serverContext);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
#ifdef QT_DEBUG 
	auto srv = qobject_cast<QUaServer*>(static_cast<QObject*>(serverContext));
	Q_CHECK_PTR(srv);
#else
	auto srv = static_cast<QUaServer*>(serverContext);
#endif // QT_DEBUG 
	// check session (triggering events create internal writes with no session)
	// NOTE : sometimes happens that !srv->m_hashSessions.contains(*sessionId)
	srv->m_currentSession = srv->m_hashSessions.contains(*sessionId) ?
		srv->m_hashSessions[*sessionId] : nullptr;
	// do not process if nobody listening
	static const QMetaMethod valueSignal = QMetaMethod::fromSignal(&QUaBaseVariable::valueChanged);
	if (var->isSignalConnected(valueSignal))
	{
		// emit value changed
		emit var->valueChanged(var->value(), !var->m_bInternalWrite);
	}
	// do not process if nobody listening
	static const QMetaMethod statusSignal = QMetaMethod::fromSignal(&QUaBaseVariable::statusCodeChanged);
	if (data->hasStatus && var->isSignalConnected(statusSignal))
	{
		// emit status changed
		emit var->statusCodeChanged(QUaStatusCode(data->status), !var->m_bInternalWrite);
	}
	// do not process if nobody listening
	static const QMetaMethod sourceSignal = QMetaMethod::fromSignal(&QUaBaseVariable::sourceTimestampChanged);
	if (data->hasSourceTimestamp && var->isSignalConnected(sourceSignal))
	{
		// emit source timestamp changed
		emit var->sourceTimestampChanged(
			QUaTypesConverter::uaVariantToQVariantScalar
				<QDateTime, UA_DateTime>(&data->sourceTimestamp), 
			!var->m_bInternalWrite
		);
	}
	// do not process if nobody listening
	static const QMetaMethod serverSignal = QMetaMethod::fromSignal(&QUaBaseVariable::serverTimestampChanged);
	if (data->hasServerTimestamp && var->isSignalConnected(serverSignal))
	{
		// emit server timestamp changed
		emit var->serverTimestampChanged(
			QUaTypesConverter::uaVariantToQVariantScalar
				<QDateTime, UA_DateTime>(&data->serverTimestamp),
			!var->m_bInternalWrite
		);
	}
	var->m_bInternalWrite = false;
}

// [STATIC] : Optionally set be the user (m_readCallback). Called before a value is requested by open62541.
// Use case is when the value is computed base don other values and it makes sense to only
// compute it when requested
void QUaBaseVariable::onRead(
	UA_Server             *server, 
	const UA_NodeId       *sessionId,
	void                  *sessionContext, 
	const UA_NodeId       *nodeId,
	void                  *nodeContext, 
	const UA_NumericRange *range,
	const UA_DataValue    *data
)
{
	Q_UNUSED(sessionContext);
	Q_UNUSED(nodeId);
	Q_UNUSED(range);
	Q_UNUSED(data);
	// get server
	void* serverContext = nullptr;
	auto st = UA_Server_getNodeContext(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), &serverContext);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
#ifdef QT_DEBUG 
	auto srv = qobject_cast<QUaServer*>(static_cast<QObject*>(serverContext));
	Q_CHECK_PTR(srv);
#else
	auto srv = static_cast<QUaServer*>(serverContext);
#endif // QT_DEBUG 
	// check session
	Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	srv->m_currentSession = srv->m_hashSessions.contains(*sessionId) ?
		srv->m_hashSessions[*sessionId] : nullptr;
	// get variable from context
#ifdef QT_DEBUG 
	auto var = qobject_cast<QUaBaseVariable*>(static_cast<QObject*>(nodeContext));
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

QUaBaseVariable::QUaBaseVariable(
	QUaServer* server
) : QUaNode(server)
{
	// [NOTE] : constructor of any QUaNode-derived class is not meant to be called by the user
	//          the constructor is called automagically by this library, and m_newNodeNodeId and
	//          m_newNodeMetaObject must be set in QUaServer before calling the constructor, as
	//          is used in QUaServer::uaConstructor
	Q_CHECK_PTR(server);
	Q_CHECK_PTR(server->m_newNodeNodeId);
	m_dataType = this->dataTypeInternal();
	// this should not be needed since stored in m_nodeId already??:
	//    const UA_NodeId &nodeId = *server->m_newNodeNodeId;
	// sets also write callback to emit onWrite signal
	setReadCallback();
#ifdef UA_ENABLE_HISTORIZING
	m_maxHistoryDataResponseSize = 1000;
#endif // UA_ENABLE_HISTORIZING
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
	return this->getValueInternal();
}

QVariant QUaBaseVariable::getValueInternal(
	const QUaTypesConverter::ArrayType& arrType
	 /* = QUaTypesConverter::ArrayType::QList*/
) const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QVariant();
	}
	// get value
	UA_ReadValueId rv;
	UA_ReadValueId_init(&rv);
	rv.nodeId      = m_nodeId;
	rv.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_DataValue value = UA_Server_read(
		m_qUaServer->m_server,
		&rv,
		UA_TIMESTAMPSTORETURN_NEITHER
	);
	// convert
	QVariant outVar = QUaTypesConverter::uaVariantToQVariant(value.value, arrType);
	// clenaup
	UA_DataValue_clear(&value);
	return outVar;
}

void QUaBaseVariable::setValue(
	const QVariant        &value, 
	const QUaStatusCode   &statusCode      /*QUaStatus::Good*/,
	const QDateTime       &sourceTimestamp /*= QDateTime()*/,
	const QDateTime       &serverTimestamp /*= QDateTime()*/,
	const QMetaType::Type &newTypeConst    /*= QMetaType::UnknownType*/
)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// get types
#define oldType m_dataType
	// get modifiable copies
	auto newValue = value;
	auto newType  = newTypeConst;

	// if new type not forced, then figure out new type from input
	if (newType == QMetaType::UnknownType)
	{
		bool isArray = QUaTypesConverter::canConvertQVariantList(value);
		if (isArray)
		{
			auto iter = value.value<QSequentialIterable>();
			if (iter.size() > 0)
			{
				QVariant innerVar = iter.at(0);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
				newType = static_cast<QMetaType::Type>( innerVar.type() );
#else
				newType = static_cast<QMetaType::Type>( innerVar.typeId() );
#endif
				if (newType == QMetaType::User) newType = static_cast<QMetaType::Type>( innerVar.userType() );
			}
			else newType = QUaTypesConverter::getQArrayType( value.typeName() );
		}
		// if scalar
		else
		{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
			newType = static_cast<QMetaType::Type>( value.type() );
#else
			newType = static_cast<QMetaType::Type>( value.typeId() );
#endif
			if (newType == QMetaType::User) newType = static_cast<QMetaType::Type>( value.userType() );
		}

		// if new type different from old type, try to keep old type
		if (newType != oldType)
		{
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
			QMetaType oldMetaType(oldType);
#endif
			if (isArray)
			{
				// can convert to old type
				QVariant innerVar;
				auto iter = value.value<QSequentialIterable>();
				if (iter.size()>0) innerVar = iter.at(0);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
				else { innerVar = QVariant( static_cast<QVariant::Type>(newType) ); }
#else
				else { innerVar = QVariant( QMetaType(newType) ); }
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
				if (innerVar.canConvert(oldType))
#else
				if (innerVar.canConvert(oldMetaType))
#endif
				{
					// convert to old type
					QVariantList listOldType;
					for (auto it = iter.begin(), itEnd = iter.end(); it != itEnd; ++it)
					{
						QVariant val = *it;
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
						val.convert(oldType);
#else
						val.convert(oldMetaType);
#endif
						listOldType.append(val);
					}

					newValue = listOldType;
					// preserve old type
					newType = oldType;
				}
			}
			// if scalar and can convert to old type
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
			else if (newValue.canConvert(oldType))
			{
				// convert to old type
				newValue.convert(oldType);
#else
			else if (newValue.canConvert(oldMetaType))
			{
				// convert to old type
				newValue.convert(oldMetaType);
#endif
				// preserve old type
				newType = oldType;
			}
			else if (!newValue.isValid())
			{
				// preserve old type
				newType = oldType;
			}
		}
	}
	// these values are maped to the same (see QUaDataType::m_custTypesByNodeId in quacustomdatatypes.cpp)
	else if (newType == QMetaType::SChar    ) { newType = QMetaType::Char; }
	else if (newType == QMetaType::LongLong ) { newType = QMetaType::Long; }
	else if (newType == QMetaType::ULongLong) { newType = QMetaType::ULong;}

	// wether new type is forced or could not be converted to old type, we need type convertion
	if (newType != oldType)
	{
		auto st = UA_Server_writeDataType(m_qUaServer->m_server,
			m_nodeId,
			QUaTypesConverter::uaTypeNodeIdFromQType(newType));
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}

	// convert to UA_Variant and set new value
#if defined(UA_GENERATED_NAMESPACE_ZERO_FULL) && !defined(OPEN62541_ISSUE3934_RESOLVED)
	UA_DataType* optDataType = nullptr;
	if (oldType == QMetaType_OptionSet)
	{
		// read type
		UA_NodeId optionSetTypeNodeId;
		auto st = UA_Server_readDataType(m_qUaServer->m_server, m_nodeId, &optionSetTypeNodeId);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		optDataType = getDataTypeFromNodeId(optionSetTypeNodeId);
		UA_NodeId_clear(&optionSetTypeNodeId);
	}
	auto uaVar = QUaTypesConverter::uaVariantFromQVariant(newValue, optDataType);
#else
	auto uaVar = QUaTypesConverter::uaVariantFromQVariant(newValue);
#endif

	// mask as internal write to avoid emitting valueChange signal on QUaBaseVariable::onWrite
	m_bInternalWrite = true;
	auto st = this->setValueInternal(
		uaVar,
		statusCode,
		sourceTimestamp, 
		serverTimestamp
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_Variant_clear(&uaVar);

	// [NOTE] do not set rank or arrayDimensions because they are permanent
	//        is better to just set array dimensions on Variant value and leave rank as ANY
	// update cache
	m_dataType = newType;
	Q_ASSERT(this->dataTypeInternal() == m_dataType);
}

QDateTime QUaBaseVariable::sourceTimestamp() const
{
	UA_ReadValueId rv;
	UA_ReadValueId_init(&rv);
	rv.nodeId      = m_nodeId;
	rv.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_DataValue value = UA_Server_read(
		m_qUaServer->m_server,
		&rv,
		UA_TIMESTAMPSTORETURN_SOURCE
	);
	QDateTime time = QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&value.sourceTimestamp);
	// clean up
	UA_DataValue_clear(&value);
	return time;
}

void QUaBaseVariable::setSourceTimestamp(const QDateTime& sourceTimestamp)
{
	// get value
	UA_ReadValueId rv;
	UA_ReadValueId_init(&rv);
	rv.nodeId      = m_nodeId;
	rv.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_DataValue value = UA_Server_read(
		m_qUaServer->m_server,
		&rv,
		UA_TIMESTAMPSTORETURN_BOTH
	);
	// set value
	UA_WriteValue wv;
	UA_WriteValue_init(&wv);
	wv.nodeId         = m_nodeId;
	wv.attributeId    = UA_ATTRIBUTEID_VALUE;
	wv.value.value    = value.value;
	wv.value.hasValue = value.hasValue;
	if (sourceTimestamp.isValid())
	{
		QUaTypesConverter::uaVariantFromQVariantScalar(sourceTimestamp, &wv.value.sourceTimestamp);
		wv.value.hasSourceTimestamp = true;
	}
	wv.value.serverTimestamp      = value.serverTimestamp     ;
	wv.value.hasServerTimestamp   = value.hasServerTimestamp  ;
	wv.value.serverPicoseconds    = value.serverPicoseconds   ;
	wv.value.sourcePicoseconds    = value.sourcePicoseconds   ;
	wv.value.hasServerPicoseconds = value.hasServerPicoseconds;
	wv.value.hasSourcePicoseconds = value.hasSourcePicoseconds;
	wv.value.status               = value.status;
	// NOTE : alternate hasStatus value to force notifying timestamp change
	// otherwise change is not sent to clients through subscription
	wv.value.hasStatus            = !value.hasStatus;
	auto st = UA_Server_write(m_qUaServer->m_server, &wv);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_Variant_clear(&value.value);
}

QDateTime QUaBaseVariable::serverTimestamp() const
{
	UA_ReadValueId rv;
	UA_ReadValueId_init(&rv);
	rv.nodeId      = m_nodeId;
	rv.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_DataValue value = UA_Server_read(
		m_qUaServer->m_server,
		&rv,
		UA_TIMESTAMPSTORETURN_SERVER
	);
	QDateTime time = QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&value.serverTimestamp);
	// clean up
	UA_DataValue_clear(&value);
	return time;
}

void QUaBaseVariable::setServerTimestamp(const QDateTime& serverTimestamp)
{
	// get value
	UA_ReadValueId rv;
	UA_ReadValueId_init(&rv);
	rv.nodeId      = m_nodeId;
	rv.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_DataValue value = UA_Server_read(
		m_qUaServer->m_server,
		&rv,
		UA_TIMESTAMPSTORETURN_BOTH
	);
	// set value
	UA_WriteValue wv;
	UA_WriteValue_init(&wv);
	wv.nodeId         = m_nodeId;
	wv.attributeId    = UA_ATTRIBUTEID_VALUE;
	wv.value.value    = value.value;
	wv.value.hasValue = value.hasValue;
	wv.value.sourceTimestamp    = value.sourceTimestamp;
	wv.value.hasSourceTimestamp = value.hasSourceTimestamp;
	if (serverTimestamp.isValid())
	{
		QUaTypesConverter::uaVariantFromQVariantScalar(serverTimestamp, &wv.value.serverTimestamp);
		wv.value.hasServerTimestamp = true;
	}
	wv.value.serverPicoseconds    = value.serverPicoseconds   ;
	wv.value.sourcePicoseconds    = value.sourcePicoseconds   ;
	wv.value.hasServerPicoseconds = value.hasServerPicoseconds;
	wv.value.hasSourcePicoseconds = value.hasSourcePicoseconds;
	wv.value.status               = value.status;
	// NOTE : alternate hasStatus value to force notifying timestamp change
	// otherwise change is not sent to clients through subscription
	wv.value.hasStatus            = !value.hasStatus;
	auto st = UA_Server_write(m_qUaServer->m_server, &wv);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_Variant_clear(&value.value);
}

QUaStatusCode QUaBaseVariable::statusCode() const
{
	UA_ReadValueId rv;
	UA_ReadValueId_init(&rv);
	rv.nodeId      = m_nodeId;
	rv.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_DataValue value = UA_Server_read(
		m_qUaServer->m_server,
		&rv,
		UA_TIMESTAMPSTORETURN_SERVER
	);
	QUaStatusCode statusCode = value.status;
	// clean up
	UA_DataValue_clear(&value);
	return statusCode;
}

void QUaBaseVariable::setStatusCode(const QUaStatusCode& statusCode)
{
	// get value
	UA_ReadValueId rv;
	UA_ReadValueId_init(&rv);
	rv.nodeId      = m_nodeId;
	rv.attributeId = UA_ATTRIBUTEID_VALUE;
	UA_DataValue value = UA_Server_read(
		m_qUaServer->m_server,
		&rv,
		UA_TIMESTAMPSTORETURN_BOTH
	);
	// set value
	UA_WriteValue wv;
	UA_WriteValue_init(&wv);
	wv.nodeId         = m_nodeId;
	wv.attributeId    = UA_ATTRIBUTEID_VALUE;
	wv.value.value    = value.value;
	wv.value.hasValue = value.hasValue;
	wv.value.sourceTimestamp      = value.sourceTimestamp     ;
	wv.value.hasSourceTimestamp   = value.hasSourceTimestamp  ;
	wv.value.serverTimestamp      = value.serverTimestamp     ;
	wv.value.hasServerTimestamp   = value.hasServerTimestamp  ;
	wv.value.serverPicoseconds    = value.serverPicoseconds   ;
	wv.value.sourcePicoseconds    = value.sourcePicoseconds   ;
	wv.value.hasServerPicoseconds = value.hasServerPicoseconds;
	wv.value.hasSourcePicoseconds = value.hasSourcePicoseconds;
	wv.value.status               = statusCode;
	wv.value.hasStatus            = true;
	auto st = UA_Server_write(m_qUaServer->m_server, &wv);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_Variant_clear(&value.value);
}

QMetaType::Type QUaBaseVariable::dataType() const
{
	return m_dataType;
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
	if (!m_qUaServer->m_hashEnums.key(outDataType).isEmpty())
	{
		UA_NodeId_clear(&outDataType);
		return QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_INT32));
	}
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	// check if type is option set, if so, return type
	if (!m_qUaServer->m_hashOptionSets.key(outDataType).isEmpty())
	{
		UA_NodeId_clear(&outDataType);
		return QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_OPTIONSET));
	}
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
	// else return converted type
	QString retNodeId = QUaTypesConverter::nodeIdToQString(outDataType);
	UA_NodeId_clear(&outDataType);
	return retNodeId;
}

void QUaBaseVariable::setDataType(const QMetaType::Type & newTypeConst)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	auto dataType = newTypeConst;
	// these values are maped to the same (see QUaDataType::m_custTypesByNodeId in quacustomdatatypes.cpp)
	if      (dataType == QMetaType::SChar    ) { dataType = QMetaType::Char; }
	else if (dataType == QMetaType::LongLong ) { dataType = QMetaType::Long; }
	else if (dataType == QMetaType::ULongLong) { dataType = QMetaType::ULong;}
	// early exit if already same
	if (dataType == m_dataType)
	{
		return;
	}
	// need to "reset" dataType before setting a new value
	auto st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// get old value
	QVariant oldValue = this->value();
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	QMetaType dataMetaType(dataType);
#endif
	// handle array
	if (QUaTypesConverter::canConvertQVariantList(oldValue))
	{
		QVariantList listConvValues;
		auto iter = oldValue.value<QSequentialIterable>();
		// loop scalar or array
		for (auto it = iter.begin(), itEnd = iter.end(); it != itEnd; ++it)
		{
			QVariant varCurr = *it;
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
			if (varCurr.canConvert(dataType)) { varCurr.convert(dataType); }
#else
			if (varCurr.canConvert(dataMetaType)) { varCurr.convert(dataMetaType); }
#endif
			else
			{
				// else set default value for type
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
				varCurr = QVariant(static_cast<QVariant::Type>(dataType));
#else
				varCurr = QVariant(dataMetaType);
#endif
			}
			// append to list of converted values
			listConvValues.append(varCurr);
		}
		// overwrite old value
		oldValue = listConvValues;
	}
	// handle scalar
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	else if (oldValue.canConvert(dataType))
	{
		// convert in place
		oldValue.convert(dataType);
	}
	else
	{
		// else set default value for type
		oldValue = QVariant(static_cast<QVariant::Type>(dataType));
	}
#else
	else if (oldValue.canConvert(dataMetaType))
	{
		// convert in place
		oldValue.convert(dataMetaType);
	}
	else
	{
		// else set default value for type
		oldValue = QVariant(dataMetaType);
	}
#endif
	// set converted or default value
	auto tmpVar = QUaTypesConverter::uaVariantFromQVariant(oldValue);
	m_bInternalWrite = true;
	st = this->setValueInternal(tmpVar);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// clean up
	UA_Variant_clear(&tmpVar);
	// set new type
	st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		QUaTypesConverter::uaTypeNodeIdFromQType(dataType));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// update cache
	m_dataType = dataType;
	Q_ASSERT(this->dataTypeInternal() == m_dataType);
}

void QUaBaseVariable::setDataTypeEnum(const QMetaEnum & metaEnum)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// compose enum name
    QString strEnumName = QStringLiteral("%1::%2").arg(
				QString::fromLatin1(metaEnum.scope()),
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
				QString::fromLatin1(metaEnum.enumName()));
#else
				QString::fromLatin1(metaEnum.name()));
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
	// get old value
	QVariant oldValue = this->value();
	// handle array
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	QMetaType metaTypeInt = QMetaType::fromType<int>();
#endif
	if (QUaTypesConverter::canConvertQVariantList(oldValue))
	{
		auto iter = oldValue.value<QSequentialIterable>();
		// get first value if any
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
		QVariant varFirst = iter.size() > 0 ? iter.at(0) : QVariant(QVariant::Int);
#else
		QVariant varFirst = iter.size() > 0 ? iter.at(0) : QVariant(metaTypeInt);
#endif
		// overwrite old value
		oldValue = varFirst;
	}
	// handle scalar
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	if (oldValue.canConvert(QMetaType::Int))
	{
		// convert in place
		oldValue.convert(QMetaType::Int);
	}
	else
	{
		// else set default value for type
		oldValue = QVariant(static_cast<QVariant::Type>(QMetaType::Int));
	}
#else
	if (oldValue.canConvert(metaTypeInt))
	{
		// convert in place
		oldValue.convert(metaTypeInt);
	}
	else
	{
		// else set default value for type
		oldValue = QVariant(metaTypeInt);
	}
#endif
	// set converted or default value
	auto tmpVar = QUaTypesConverter::uaVariantFromQVariant(oldValue);
	m_bInternalWrite = true;
	st = this->setValueInternal(tmpVar);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// clean up
	UA_Variant_clear(&tmpVar);
	// change data type
	st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		enumTypeNodeId);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// update cache
	m_dataType = QMetaType::Int;
	Q_ASSERT(this->dataTypeInternal() == m_dataType);
}

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
bool QUaBaseVariable::setDataTypeOptionSet(const QString& strOptionSetName)
{
	// check if exists in server's hash
	if (!m_qUaServer->m_hashOptionSets.contains(strOptionSetName))
	{
		return false;
	}
	// get option set nodeId
	UA_NodeId optionSetTypeNodeId = m_qUaServer->m_hashOptionSets.value(strOptionSetName);
	// call internal method
	this->setDataTypeOptionSet(optionSetTypeNodeId);
	// success
	return true;
}

// TODO : update to specific type and handle conversions there
void QUaBaseVariable::setDataTypeOptionSet(const UA_NodeId& optionSetTypeNodeId)
{
	// need to "reset" dataType before setting a new value
	auto st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE));
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// get old value
	QVariant oldValue = this->value();
	// handle array
	if (QUaTypesConverter::canConvertQVariantList(oldValue))
	{
		auto iter = oldValue.value<QSequentialIterable>();
		// get first value if any
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
		QVariant varFirst = iter.size() > 0 ? iter.at(0) : QVariant(QVariant::ULongLong);
#else
		QVariant varFirst = iter.size() > 0 ? iter.at(0) : QVariant( QMetaType(QMetaType::ULongLong) );
#endif
		// overwrite old value
		oldValue = varFirst;
	}
	// handle scalar
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	if (oldValue.canConvert(QVariant::ULongLong))
#else
	if (oldValue.canConvert( QMetaType(QMetaType::ULongLong) ))
#endif
	{
		// convert in place
		oldValue = QVariant::fromValue(QUaOptionSet(oldValue.toULongLong()));
	}
	else
	{
		// else set default value for type
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
		oldValue = QVariant(static_cast<QVariant::Type>(QMetaType_OptionSet));
#else
		oldValue = QVariant( QMetaType(QMetaType_OptionSet) );
#endif
	}
	// set converted or default value
#ifndef OPEN62541_ISSUE3934_RESOLVED
	UA_DataType* optDataType = getDataTypeFromNodeId(optionSetTypeNodeId);
	auto tmpVar = QUaTypesConverter::uaVariantFromQVariant(oldValue, optDataType);
#else
	auto tmpVar = QUaTypesConverter::uaVariantFromQVariant(oldValue);
#endif // !OPEN62541_ISSUE3934_RESOLVED
	m_bInternalWrite = true;
	st = this->setValueInternal(tmpVar);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// clean up
	UA_Variant_clear(&tmpVar);
	// change data type
	st = UA_Server_writeDataType(m_qUaServer->m_server,
		m_nodeId,
		optionSetTypeNodeId
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// update cache
	m_dataType = QMetaType_OptionSet;
	Q_ASSERT(this->dataTypeInternal() == m_dataType);
}
#endif

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
	if (!m_qUaServer->m_hashEnums.key(outDataType).isEmpty())
	{
		UA_NodeId_clear(&outDataType);
		return QMetaType::Int;
	}
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	// check if type is option set, if so, return type QMetaType_OptionSet
	if (!m_qUaServer->m_hashOptionSets.key(outDataType).isEmpty())
	{
		UA_NodeId_clear(&outDataType);
		return QMetaType_OptionSet;
	}
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
	// else return converted type
	QMetaType::Type type = QUaTypesConverter::uaTypeNodeIdToQType(&outDataType);
	UA_NodeId_clear(&outDataType);
	return type;
}

UA_StatusCode QUaBaseVariable::setValueInternal(
	const UA_Variant    &value,
	const UA_StatusCode &status,
	const QDateTime     &sourceTimestamp, 
	const QDateTime     &serverTimestamp)
{
	// set value
	UA_WriteValue wv;
	UA_WriteValue_init(&wv);
	wv.nodeId         = m_nodeId;
	wv.attributeId    = UA_ATTRIBUTEID_VALUE;
	wv.value.value    = value;
	wv.value.hasValue = 1;
	if (sourceTimestamp.isValid())
	{
		QUaTypesConverter::uaVariantFromQVariantScalar(sourceTimestamp, &wv.value.sourceTimestamp);
		wv.value.hasSourceTimestamp = true;
	}
	if (serverTimestamp.isValid())
	{
		QUaTypesConverter::uaVariantFromQVariantScalar(serverTimestamp, &wv.value.serverTimestamp);
		wv.value.hasServerTimestamp = true;
	}
	wv.value.serverPicoseconds    = 0;
	wv.value.sourcePicoseconds    = 0;
	wv.value.hasServerPicoseconds = false;
	wv.value.hasSourcePicoseconds = false;
	wv.value.status               = status;
	wv.value.hasStatus            = true;
	auto st = UA_Server_write(m_qUaServer->m_server, &wv);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	return st;
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
	auto data = static_cast<quint32*>(outArrayDimensions.data);
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

#ifdef UA_ENABLE_HISTORIZING
void QUaBaseVariable::setHistorizing(const bool& historizing)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set historizing
	auto st = UA_Server_writeHistorizing(m_qUaServer->m_server, m_nodeId, historizing);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	if (!historizing)
	{
		return;
	}
	// check if historizing already set
	auto gathering = m_qUaServer->getGathering();
	auto psetting  = gathering.getHistorizingSetting(
		m_qUaServer->m_server,
		gathering.context,
		&m_nodeId
	);
	if (psetting)
	{
		return;
	}
	// setup historizing 
	UA_HistorizingNodeIdSettings setting;
	setting.historizingBackend         = QUaHistoryBackend::m_historUaBackend;
	setting.maxHistoryDataResponseSize = m_maxHistoryDataResponseSize; // max size client can ask for
	setting.historizingUpdateStrategy  = UA_HISTORIZINGUPDATESTRATEGY_VALUESET; // when value updated or polling
	st = gathering.registerNodeId(m_qUaServer->m_server, gathering.context, &m_nodeId, setting);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}
quint64 QUaBaseVariable::maxHistoryDataResponseSize() const
{
	return m_maxHistoryDataResponseSize;
}
void QUaBaseVariable::setMaxHistoryDataResponseSize(const quint64& maxHistoryDataResponseSize)
{
	// set internal value (put a minimum of 50 just in case)
	m_maxHistoryDataResponseSize = (std::max)(static_cast<quint64>(50), maxHistoryDataResponseSize);
	// check if historizing already set
	auto gathering = m_qUaServer->getGathering();
	UA_NodeIdStoreContext* ctx = (UA_NodeIdStoreContext*)gathering.context;
	UA_NodeIdStoreContextItem_gathering_default* item = getNodeIdStoreContextItem_gathering_default(ctx, &m_nodeId);
	if (!item) {
		return;
	}
	item->setting.maxHistoryDataResponseSize = m_maxHistoryDataResponseSize; // max size client can ask for
}
#endif // UA_ENABLE_HISTORIZING

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

#ifdef UA_ENABLE_HISTORIZING
bool QUaBaseVariable::readHistoryAccess() const
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue = this->accessLevel();
	return accessLevel.bits.bHistoryRead;
}

void QUaBaseVariable::setReadHistoryAccess(const bool& readHistoryAccess)
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue = this->accessLevel();
	accessLevel.bits.bHistoryRead = readHistoryAccess;
	this->setAccessLevel(accessLevel.intValue);
}

bool QUaBaseVariable::writeHistoryAccess() const
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue = this->accessLevel();
	return accessLevel.bits.bHistoryWrite;
}

void QUaBaseVariable::setWriteHistoryAccess(const bool& bHistoryWrite)
{
	QUaAccessLevel accessLevel;
	accessLevel.intValue = this->accessLevel();
	accessLevel.bits.bHistoryWrite = bHistoryWrite;
	this->setAccessLevel(accessLevel.intValue);
}
#endif // UA_ENABLE_HISTORIZING

// [STATIC]
qint32 QUaBaseVariable::GetValueRankFromQVariant(const QVariant & varValue)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	auto originalType = static_cast<QMetaType::Type>( varValue.type() );
#else
	auto originalType = static_cast<QMetaType::Type>( varValue.typeId() );
#endif
	if (originalType == QMetaType::UnknownType)
	{
		return UA_VALUERANK_ANY;
	}
	else if (QUaTypesConverter::canConvertQVariantList(varValue))
	{
		return UA_VALUERANK_ONE_DIMENSION;
	}
	// scalar is default
	return UA_VALUERANK_SCALAR;
}

// [STATIC]
QVector<quint32> QUaBaseVariable::GetArrayDimensionsFromQVariant(const QVariant & varValue)
{
	if (QUaTypesConverter::canConvertQVariantList(varValue))
	{
		auto iter = varValue.value<QSequentialIterable>();
		auto size = (quint32)iter.size();
		return QVector<quint32>() << size;
	}
	// default arrayDimensionsSize == 0
	return QVector<quint32>();
}
