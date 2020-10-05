#include <QUaServer>

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL

UA_StatusCode QUaServer::addOptionSetValues(UA_Server* server, UA_NodeId* parent, const UA_UInt32 numOptionSetValues, const QUaLocalizedText* optionSetValues)
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
	arrayDimensions[0]           = numOptionSetValues;
	attr.arrayDimensions         = &arrayDimensions[0];
	attr.dataType                = UA_NODEID_NUMERIC(0, UA_NS0ID_LOCALIZEDTEXT);
	// create array of option set values
	UA_LocalizedText * valueOptionSet = numOptionSetValues > 0 ? (UA_LocalizedText*)UA_malloc(sizeof(UA_LocalizedText) * numOptionSetValues) : nullptr;
	for (size_t i = 0; i < numOptionSetValues; i++)
	{
		UA_init(&valueOptionSet[i], &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
		valueOptionSet[i] = optionSetValues[i].toUaLocalizedText();
	}
	// create variant with array of option set values
	UA_Variant_setArray(&attr.value, valueOptionSet, (UA_Int32)numOptionSetValues, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
	attr.displayName   = UA_LOCALIZEDTEXT((char*)"", (char*)"OptionSetValues");
	attr.description   = UA_LOCALIZEDTEXT((char*)"", (char*)"");
	attr.writeMask     = 0;
	attr.userWriteMask = 0;
	// add variable with array of option set values
	UA_NodeId optionSetValuesNodeId;
	retVal |= UA_Server_addVariableNode(server,
                                        UA_NODEID_NULL, 
                                        *parent, 
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                        UA_QUALIFIEDNAME (0, (char*)"OptionSetValues"),
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), 
                                        attr, 
                                        NULL, 
                                        &optionSetValuesNodeId);
	Q_ASSERT(retVal == UA_STATUSCODE_GOOD);
	// make mandatory
	retVal |= UA_Server_addReference(server,
					                 optionSetValuesNodeId,
					                 UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
					                 UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
					                 true);
	Q_ASSERT(retVal == UA_STATUSCODE_GOOD);
	// success
	return retVal;
}

void QUaServer::registerOptionSet(const QString& strOptionSetName, const QUaOptionSetMap& optionSetMap, const QUaNodeId& nodeId)
{
	// check if already exists
	if (m_hashOptionSets.contains(strOptionSetName))
	{
		return;
	}
	// get c-compatible name
	QByteArray byteOptionSetName = strOptionSetName.toUtf8();
	char* charOptionSetName = byteOptionSetName.data();
	// register enum as new ua data type
	UA_DataTypeAttributes ddaatt = UA_DataTypeAttributes_default;
	ddaatt.description = UA_LOCALIZEDTEXT((char*)(""), charOptionSetName);
	ddaatt.displayName = UA_LOCALIZEDTEXT((char*)(""), charOptionSetName);
	// check if requested node id defined
	if (!nodeId.isNull())
	{
		// check if requested node id exists
		bool isUsed = this->isNodeIdUsed(nodeId);
		Q_ASSERT_X(!isUsed, "QUaServer::registerOptionSet", "Requested NodeId already exists");
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
		reqNodeId = UA_NODEID_STRING_ALLOC(1, charOptionSetName);
	}
	auto st = UA_Server_addDataTypeNode(m_server,
		reqNodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_OPTIONSET),
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		UA_QUALIFIEDNAME(1, charOptionSetName),
		ddaatt,
		NULL,
		NULL);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// create vector of bit values (names)
	auto bits = optionSetMap.keys();
	auto maxBit = *(std::max_element)(bits.begin(), bits.end()) + 1; // NOTE : +1 because loop b < maxBit
	if (maxBit > 64)
	{
		Q_ASSERT_X(false, "QUaServer::registerOptionSet", "Maximum option set size is 64bits");		
	}
	maxBit = 64;
	auto vectOptionSetValues = QVector<QUaLocalizedText>(maxBit);
	for (int b = 0; b < (int)maxBit; b++)
	{
		// if name for bit defined by user, then use it, else set default
		if (optionSetMap.contains(b))
		{
			vectOptionSetValues[b] = {
				optionSetMap[b].text(),
				optionSetMap[b].locale()
			};
		}
		else
		{
			vectOptionSetValues[b] = { "", "" };
		}
	}
	Q_ASSERT(vectOptionSetValues.count() == maxBit);
	st = QUaServer::addOptionSetValues(m_server, &reqNodeId, maxBit, vectOptionSetValues.data());
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// finally append to map
	m_hashOptionSets.insert(strOptionSetName, reqNodeId);
}

bool QUaServer::isOptionSetRegistered(const QString& strOptionSetName) const
{
	return m_hashOptionSets.contains(strOptionSetName);
}

QUaOptionSetMap QUaServer::optionSetMap(const QString& strOptionSetName) const
{
	// TODO
	Q_UNUSED(strOptionSetName);
	return QUaOptionSetMap();
}

void QUaServer::setOptionSetMap(const QString& strOptionSetName, const QUaOptionSetMap& optionSetMap)
{
	// TODO
	Q_UNUSED(strOptionSetName);
	Q_UNUSED(optionSetMap);
}

void QUaServer::updateOptionSetEntry(const QString& strOptionSetName, const QUaOptionSetBit& optionSetBit, const QUaLocalizedText& optionSetEntry)
{
	// TODO
	Q_UNUSED(strOptionSetName);
	Q_UNUSED(optionSetBit);
	Q_UNUSED(optionSetEntry);
}

void QUaServer::removeOptionSetEntry(const QString& strOptionSetName, const QUaOptionSetBit& optionSetBit)
{
	// TODO
	Q_UNUSED(strOptionSetName);
	Q_UNUSED(optionSetBit);
}

UA_NodeId QUaServer::optionSetValuesNodeId(const UA_NodeId& optionSetNodeId) const
{
	// TODO
	Q_UNUSED(optionSetNodeId);
	return UA_NodeId();
}

UA_Variant QUaServer::optionSetValues(const UA_NodeId& optionSetNodeId) const
{
	// TODO
	Q_UNUSED(optionSetNodeId);
	return UA_Variant();
}

void QUaServer::updateOptionSet(const UA_NodeId& optionSetNodeId, const QUaOptionSetMap& optionSetMap)
{
	// TODO
	Q_UNUSED(optionSetNodeId);
	Q_UNUSED(optionSetMap);
}

#endif