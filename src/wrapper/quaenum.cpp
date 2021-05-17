#include <QUaServer>

UA_StatusCode QUaServer::addEnumValues(UA_Server * server, UA_NodeId * parent, const UA_UInt32 numEnumValues, const QOpcUaEnumValue * enumValues)
{
	// setup variable attrs
	UA_StatusCode retVal         = UA_STATUSCODE_GOOD;
	UA_VariableAttributes attr   = UA_VariableAttributes_default;
	attr.minimumSamplingInterval = 0.000000;
	attr.userAccessLevel         = 1;
	attr.accessLevel             = 1;
	attr.valueRank               = 1;
	attr.arrayDimensionsSize     = 1;
	UA_UInt32 arrayDimensions[1];
	arrayDimensions[0]           = numEnumValues; // WHY 0; ?
	attr.arrayDimensions         = &arrayDimensions[0];
	attr.dataType                = UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMVALUETYPE);
	// create array of enum values
	UA_EnumValueType * valueEnum = numEnumValues > 0 ? (UA_EnumValueType *)UA_malloc(sizeof(UA_EnumValueType) * numEnumValues) : nullptr;
	for (size_t i = 0; i < numEnumValues; i++)
	{
		UA_init(&valueEnum[i], &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
		valueEnum[i].value       = enumValues[i].Value;
		valueEnum[i].displayName = enumValues[i].DisplayName;
		valueEnum[i].description = enumValues[i].Description;
	}
	// create variant with array of enum values
	UA_Variant_setArray(&attr.value, valueEnum, (UA_Int32)numEnumValues, &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
	attr.displayName   = UA_LOCALIZEDTEXT((char*)"", (char*)"EnumValues");
	attr.description   = UA_LOCALIZEDTEXT((char*)"", (char*)"");
	attr.writeMask     = 0;
	attr.userWriteMask = 0;
	// add variable with array of enum values
	UA_NodeId enumValuesNodeId;
	retVal |= UA_Server_addVariableNode(server,
                                        UA_NODEID_NULL, 
                                        *parent, 
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                        UA_QUALIFIEDNAME (0, (char*)"EnumValues"),
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), 
                                        attr, 
                                        NULL, 
                                        &enumValuesNodeId);
	Q_ASSERT(retVal == UA_STATUSCODE_GOOD);
	// make mandatory
	retVal |= UA_Server_addReference(server,
					                 enumValuesNodeId,
					                 UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
					                 UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
					                 true);
	Q_ASSERT(retVal == UA_STATUSCODE_GOOD);
	// success
	return retVal;
}

void QUaServer::registerEnum(const QString& strEnumName, const QUaEnumMap& enumMap, const QUaNodeId& nodeId)
{
	// check if already exists
	if (m_hashEnums.contains(strEnumName))
	{
		return;
	}
	// get c-compatible name
	QByteArray byteEnumName = strEnumName.toUtf8();
	char* charEnumName = byteEnumName.data();
	// register enum as new ua data type
	UA_DataTypeAttributes ddaatt = UA_DataTypeAttributes_default;
	ddaatt.description = UA_LOCALIZEDTEXT((char*)(""), charEnumName);
	ddaatt.displayName = UA_LOCALIZEDTEXT((char*)(""), charEnumName);
	// check if requested node id defined
	if (!nodeId.isNull())
	{
		// check if requested node id exists
		bool isUsed = this->isNodeIdUsed(nodeId);
		Q_ASSERT_X(!isUsed, "QUaServer::registerEnum", "Requested NodeId already exists");
		if (isUsed)
		{
			return;
		}
	}
	// if null, then assign one because is feaking necessary
	// https://github.com/open62541/open62541/issues/2584
	UA_NodeId reqNodeId = nodeId;
	if (nodeId.isNull())
	{
		// [IMPORTANT] : _ALLOC version is necessary
		reqNodeId = UA_NODEID_STRING_ALLOC(1, charEnumName);
	}
	auto st = UA_Server_addDataTypeNode(m_server,
		reqNodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMERATION),
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		UA_QUALIFIEDNAME(1, charEnumName),
		ddaatt,
		NULL,
		NULL);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// create vector of enum values
	QVector<QOpcUaEnumValue> vectEnumValues;
	QUaEnumMapIter i(enumMap);
	while (i.hasNext())
	{
		i.next();
		vectEnumValues.append({
			(UA_Int64)i.key(),
			i.value().displayName,
			i.value().description
			});
	}
	st = QUaServer::addEnumValues(m_server, &reqNodeId, vectEnumValues.count(), vectEnumValues.data());
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// finally append to map
	m_hashEnums.insert(strEnumName, reqNodeId);
}

bool QUaServer::isEnumRegistered(const QString& strEnumName) const
{
	return m_hashEnums.contains(strEnumName);
}

// NOTE : expensive, creates a copy
QUaEnumMap QUaServer::enumMap(const QString& strEnumName) const
{
	auto retMap = QUaEnumMap();
	// if it does not exist, return empty
	if (!m_hashEnums.contains(strEnumName))
	{
		return retMap;
	}
	// read enum map
	auto enumNodeId = m_hashEnums[strEnumName];
	// get enum values as ua variant
	auto enumValues = this->enumValues(enumNodeId);
	// get start of array
	UA_EnumValueType* enumArr = static_cast<UA_EnumValueType*>(enumValues.data);
	for (size_t i = 0; i < enumValues.arrayLength; i++)
	{
		UA_EnumValueType* enumVal = &enumArr[i];
		retMap.insert(enumVal->value, { enumVal->displayName, enumVal->description });
	}
	// clean up
	UA_Variant_clear(&enumValues);
	// return
	return retMap;
}

void QUaServer::setEnumMap(const QString& strEnumName, const QUaEnumMap& enumMap)
{
	// if it does not exist, create it with one entry
	if (!m_hashEnums.contains(strEnumName))
	{
		this->registerEnum(strEnumName, enumMap);
		return;
	}
	// else update enum
	auto & enumNodeId = m_hashEnums[strEnumName];
	this->updateEnum(enumNodeId, enumMap);
}

// NOTE : expensive because it needs to copy old array into new, update new and delete old array,
//        to insert multiple elements at once use QUaServer::updateEnum
void QUaServer::updateEnumEntry(const QString& strEnumName, const QUaEnumKey& enumValue, const QUaEnumEntry& enumEntry)
{
	// if it does not exist, create it with one entry
	if (!m_hashEnums.contains(strEnumName))
	{
		this->registerEnum(strEnumName, { {enumValue, enumEntry} });
		return;
	}
	// else update enum
	auto & enumNodeId = m_hashEnums[strEnumName];
	// get old map
	auto mapValues = this->enumMap(strEnumName);
	// update old map
	mapValues[enumValue] = enumEntry;
	this->updateEnum(enumNodeId, mapValues);
}

// NOTE : expensive because it needs to copy old array into new, update new and delete old array,
//        to insert multiple elements at once use QUaServer::updateEnum with empty argument
void QUaServer::removeEnumEntry(const QString& strEnumName, const QUaEnumKey& enumValue)
{
	// if it does not exist, do nothing
	if (!m_hashEnums.contains(strEnumName))
	{
		return;
	}
	// else update enum
	auto & enumNodeId = m_hashEnums[strEnumName];
	// get old map
	auto mapValues = this->enumMap(strEnumName);
	// update old map
	mapValues.remove(enumValue);
	this->updateEnum(enumNodeId, mapValues);
}

void QUaServer::registerEnum(const QMetaEnum& metaEnum, const QUaNodeId& nodeId/* = ""*/)
{
	// compose enum name
#if QT_VERSION >= 0x051200
	QString strBrowseName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
#else
	QString strBrowseName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.name());
#endif
	// compose values
	QUaEnumMap mapEnum;
	for (int i = 0; i < metaEnum.keyCount(); i++)
	{
		mapEnum.insert(metaEnum.value(i), { metaEnum.key(i), "" });
	}
	// call other method
	this->registerEnum(strBrowseName, mapEnum, nodeId);
}

// NOTE : need to cleanup result after calling this method
UA_NodeId QUaServer::enumValuesNodeId(const UA_NodeId& enumNodeId) const
{
	// make ua browse
	UA_BrowseDescription* bDesc = UA_BrowseDescription_new();
	UA_NodeId_copy(&enumNodeId, &bDesc->nodeId); // from child
	bDesc->browseDirection = UA_BROWSEDIRECTION_FORWARD;
	bDesc->includeSubtypes = true;
	bDesc->resultMask = UA_BROWSERESULTMASK_REFERENCETYPEID;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(m_server, 0, bDesc);
	Q_ASSERT(bRes.statusCode == UA_STATUSCODE_GOOD);
	Q_ASSERT(bRes.referencesSize == 1);
	UA_ReferenceDescription rDesc = bRes.references[0];
	UA_NodeId valuesNodeId/* = rDesc.nodeId.nodeId*/;
	UA_NodeId_copy(&rDesc.nodeId.nodeId, &valuesNodeId);
	// cleanup
    UA_BrowseDescription_clear(bDesc);
	UA_BrowseDescription_delete(bDesc);
    UA_BrowseResult_clear(&bRes);
	// return
	return valuesNodeId;
}

UA_Variant QUaServer::enumValues(const UA_NodeId& enumNodeId) const
{
	UA_NodeId valuesNodeId = this->enumValuesNodeId(enumNodeId);
	// read value
	UA_Variant outValue;
	auto st = UA_Server_readValue(m_server, valuesNodeId, &outValue);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	Q_ASSERT(!UA_Variant_isScalar(&outValue));
	// cleanup
	UA_NodeId_clear(&valuesNodeId);
	// return
	return outValue;
}

void QUaServer::updateEnum(const UA_NodeId& enumNodeId, const QUaEnumMap& mapEnum)
{
	// get enum values as ua variant
	auto enumValuesNodeId = this->enumValuesNodeId(enumNodeId);
	auto enumValues = this->enumValues(enumNodeId);
	// delete old ua enum
	UA_Variant_clear(&enumValues);
	// re-create array of enum values
	UA_EnumValueType* valueEnum = nullptr;
	if (mapEnum.count() > 0)
	{
		valueEnum = (UA_EnumValueType*)UA_malloc(sizeof(UA_EnumValueType) * mapEnum.count());
	}
	auto listKeys = mapEnum.keys();
	for (int i = 0; i < mapEnum.count(); i++)
	{
		UA_init(&valueEnum[i], &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
		valueEnum[i].value = (UA_Int64)listKeys.at(i);
		valueEnum[i].displayName = mapEnum[listKeys.at(i)].displayName;
		valueEnum[i].description = mapEnum[listKeys.at(i)].description;
	}
	// create variant with array of enum values
	UA_Variant_init(&enumValues);
	UA_Variant_setArray(&enumValues, valueEnum, (UA_Int32)mapEnum.count(), &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
	// set new ua enum value
	auto st = UA_Server_writeValue(m_server, enumValuesNodeId, enumValues);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// cleanup
	UA_NodeId_clear(&enumValuesNodeId);
	for (int i = 0; i < mapEnum.count(); i++)
	{
		UA_EnumValueType_clear(&valueEnum[i]);
	}
	UA_free(valueEnum);
}
