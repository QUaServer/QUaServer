#include "quabaseevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

#include "quaserver_anex.h"

QUaBaseEvent::QUaBaseEvent(
	QUaServer *server
) : QUaBaseObject(server)
{
	m_sourceNodeId = UA_NODEID_NULL;
	// set event type definition
	this->setEventType(this->typeDefinitionNodeId());
	// set a default parent to this is not dangling
	if (!this->parent())
	{
		this->setParent(server);
	}
}

QUaBaseEvent::~QUaBaseEvent()
{
	UA_NodeId_clear(&m_sourceNodeId);
}

QByteArray QUaBaseEvent::eventId() const
{
	return const_cast<QUaBaseEvent*>(this)->getEventId()->value().toByteArray();
}

void QUaBaseEvent::setEventId(const QByteArray& eventId)
{
    this->getEventId()->setValue(eventId);
}

QString QUaBaseEvent::eventType() const
{
	return const_cast<QUaBaseEvent*>(this)->getEventType()->value().toString();
}

void QUaBaseEvent::setEventType(const QString& eventTypeNodeId)
{
	return this->getEventType()->setValue(
		eventTypeNodeId,
		QUaStatusCode(),
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

QString QUaBaseEvent::sourceNode() const
{
	return const_cast<QUaBaseEvent*>(this)->getSourceNode()->value().toString();
}

void QUaBaseEvent::setSourceNode(const QString& sourceNodeId)
{
	UA_NodeId srcNodeId = QUaTypesConverter::nodeIdFromQString(sourceNodeId);
	// set cache
	UA_NodeId_clear(&m_sourceNodeId);
	m_sourceNodeId = srcNodeId;
	// set internally
	return this->getSourceNode()->setValue(
		sourceNodeId,
		QUaStatusCode(),
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

void QUaBaseEvent::setSourceNodeByRef(const QUaNode* sourceNode)
{
    this->setSourceNode(sourceNode ? sourceNode->nodeId()      : "");
    this->setSourceName(sourceNode ? sourceNode->displayName() : "");
}

QString QUaBaseEvent::sourceName() const
{
	return const_cast<QUaBaseEvent*>(this)->getSourceName()->value().toString();
}

void QUaBaseEvent::setSourceName(const QString & strSourceName)
{
	this->getSourceName()->setValue(strSourceName);
}

QDateTime QUaBaseEvent::time() const
{
	return const_cast<QUaBaseEvent*>(this)->getTime()->value().toDateTime().toUTC();
}

void QUaBaseEvent::setTime(const QDateTime & dateTime)
{
	this->getTime()->setValue(dateTime.toUTC());
}

QDateTime QUaBaseEvent::receiveTime() const
{
	return const_cast<QUaBaseEvent*>(this)->getReceiveTime()->value().toDateTime().toUTC();
}

void QUaBaseEvent::setReceiveTime(const QDateTime& dateTime)
{
    this->getReceiveTime()->setValue(dateTime.toUTC());
}

// NOTE : removed because is optional and open62541 now does not add it
QTimeZone QUaBaseEvent::localTime() const
{
	return qvariant_cast<QTimeZone>(const_cast<QUaBaseEvent*>(this)->getLocalTime()->value());
}

void QUaBaseEvent::setLocalTime(const QTimeZone & localTimeZone)
{
	this->getLocalTime()->setValue(
		QVariant::fromValue(localTimeZone), 
		QUaStatusCode(),
		QDateTime(), 
		QDateTime(), 
		METATYPE_TIMEZONEDATATYPE
	);
}

QString QUaBaseEvent::message() const
{
	return const_cast<QUaBaseEvent*>(this)->getMessage()->value().toString();
}

void QUaBaseEvent::setMessage(const QString & strMessage)
{
	this->getMessage()->setValue(
		strMessage, 
		QUaStatusCode(),
		QDateTime(),
		QDateTime(),
		METATYPE_LOCALIZEDTEXT
	);
}

quint16 QUaBaseEvent::severity() const
{
	return (quint16)const_cast<QUaBaseEvent*>(this)->getSeverity()->value().toUInt();
}

void QUaBaseEvent::setSeverity(const quint16 & intSeverity)
{
	this->getSeverity()->setValue(intSeverity);
}

void QUaBaseEvent::trigger()
{
    this->setEventId(QUaBaseEvent::generateEventId());
    this->triggerRaw();
}

QUaProperty * QUaBaseEvent::getEventId() 
{
	return this->browseChild<QUaProperty>("EventId");
}

QUaProperty* QUaBaseEvent::getEventType()
{
	return this->browseChild<QUaProperty>("EventType");
}

QUaProperty* QUaBaseEvent::getSourceNode()
{
	return this->browseChild<QUaProperty>("SourceNode");
}

QUaProperty* QUaBaseEvent::getSourceName()
{
	return this->browseChild<QUaProperty>("SourceName");
}

QUaProperty* QUaBaseEvent::getTime()
{
	return this->browseChild<QUaProperty>("Time");
}

QUaProperty* QUaBaseEvent::getReceiveTime()
{
	return this->browseChild<QUaProperty>("ReceiveTime");
}

QUaProperty* QUaBaseEvent::getLocalTime()
{
	return this->browseChild<QUaProperty>("LocalTime", true);
}

QUaProperty* QUaBaseEvent::getMessage()
{
	return this->browseChild<QUaProperty>("Message");
}

QUaProperty* QUaBaseEvent::getSeverity()
{
	return this->browseChild<QUaProperty>("Severity");
}

QByteArray QUaBaseEvent::generateEventId()
{
	UA_ByteString source = QUaBaseEvent::generateEventIdInternal();
	QByteArray ret = QUaTypesConverter::uaVariantToQVariantScalar<QByteArray, UA_ByteString>(&source);
	UA_ByteString_clear(&source);
	return ret;
}

UA_ByteString QUaBaseEvent::generateEventIdInternal()
{
	UA_ByteString ret = UA_BYTESTRING_NULL;
	auto st = UA_Event_generateEventId(&ret);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return ret;
}

static const UA_NodeId objectsFolderId = { 0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_OBJECTSFOLDER} };
#define EMIT_REFS_ROOT_COUNT 4
static const UA_NodeId emitReferencesRoots[EMIT_REFS_ROOT_COUNT] =
{ {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_ORGANIZES}},
 {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASCOMPONENT}},
 {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASEVENTSOURCE}},
 {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASNOTIFIER}} };


static UA_Boolean
isValidEvent(UA_Server* server, const UA_NodeId* validEventParent,
    const UA_NodeId* eventId) {
    /* find the eventType variableNode */
    UA_QualifiedName findName = UA_QUALIFIEDNAME(0, (char*)"EventType");
    UA_BrowsePathResult bpr = browseSimplifiedBrowsePath(server, *eventId, 1, &findName);
    if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1) {
        UA_BrowsePathResult_clear(&bpr);
        return false;
    }

    /* Get the EventType Property Node */
    UA_Variant tOutVariant;
    UA_Variant_init(&tOutVariant);

    /* Read the Value of EventType Property Node (the Value should be a NodeId) */
    UA_StatusCode retval = readWithReadValue(server, &bpr.targets[0].targetId.nodeId,
        UA_ATTRIBUTEID_VALUE, &tOutVariant);
    if (retval != UA_STATUSCODE_GOOD ||
        !UA_Variant_hasScalarType(&tOutVariant, &UA_TYPES[UA_TYPES_NODEID])) {
        UA_BrowsePathResult_clear(&bpr);
        return false;
    }

    const UA_NodeId* tEventType = (UA_NodeId*)tOutVariant.data;

    /* check whether the EventType is a Subtype of CondtionType
     * (Part 9 first implementation) */
    UA_NodeId conditionTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE);
    UA_NodeId hasSubtypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE);

    if (UA_NodeId_equal(validEventParent, &conditionTypeId) &&
        isNodeInTree(server, tEventType,
            &conditionTypeId, &hasSubtypeId, 1)) {
        UA_BrowsePathResult_deleteMembers(&bpr);
        UA_Variant_clear(&tOutVariant);
        return true;
    }

    /*EventType is not a Subtype of CondtionType
     *(ConditionId Clause won't be present in Events, which are not Conditions)*/
     /* check whether Valid Event other than Conditions */
    UA_NodeId baseEventTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
    UA_Boolean isSubtypeOfBaseEvent = isNodeInTree(server, tEventType,
        &baseEventTypeId, &hasSubtypeId, 1);

    UA_BrowsePathResult_clear(&bpr);
    UA_Variant_clear(&tOutVariant);
    return isSubtypeOfBaseEvent;
}

/* Part 4: 7.4.4.5 SimpleAttributeOperand
 * The clause can point to any attribute of nodes. Either a child of the event
 * node and also the event type. */
static UA_StatusCode
resolveSimpleAttributeOperand(UA_Server* server, UA_Session* session, const UA_NodeId* origin,
    const UA_SimpleAttributeOperand* sao, UA_Variant* value) {
    /* Prepare the ReadValueId */
    UA_ReadValueId rvi;
    UA_ReadValueId_init(&rvi);
    rvi.indexRange = sao->indexRange;
    rvi.attributeId = sao->attributeId;

    /* If this list (browsePath) is empty the Node is the instance of the
     * TypeDefinition. */
    if (sao->browsePathSize == 0) {
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
        //TODO check for Branches! One Condition could have multiple Branches
        // Set ConditionId
        UA_NodeId conditionTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE);
        if (UA_NodeId_equal(&sao->typeDefinitionId, &conditionTypeId)) {
            UA_NodeId conditionId;
            UA_StatusCode retval = UA_getConditionId(server, origin, &conditionId);
            if (retval != UA_STATUSCODE_GOOD)
                return retval;

            rvi.nodeId = conditionId;
        }
        else
            rvi.nodeId = sao->typeDefinitionId;
#else
        rvi.nodeId = sao->typeDefinitionId;
#endif /*UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS*/
        UA_DataValue v = UA_Server_readWithSession(server, session, &rvi,
            UA_TIMESTAMPSTORETURN_NEITHER);
        if (v.status == UA_STATUSCODE_GOOD && v.hasValue)
            *value = v.value;
        return v.status;
    }

    /* Resolve the browse path */
    UA_BrowsePathResult bpr =
        browseSimplifiedBrowsePath(server, *origin, sao->browsePathSize, sao->browsePath);
    if (bpr.targetsSize == 0 && bpr.statusCode == UA_STATUSCODE_GOOD)
        bpr.statusCode = UA_STATUSCODE_BADNOTFOUND;
    if (bpr.statusCode != UA_STATUSCODE_GOOD) {
        UA_StatusCode retval = bpr.statusCode;
        UA_BrowsePathResult_clear(&bpr);
        return retval;
    }

    /* Read the first matching element. Move the value to the output. */
    rvi.nodeId = bpr.targets[0].targetId.nodeId;
    UA_DataValue v = UA_Server_readWithSession(server, session, &rvi,
        UA_TIMESTAMPSTORETURN_NEITHER);
    if (v.status == UA_STATUSCODE_GOOD && v.hasValue)
        *value = v.value;

    UA_BrowsePathResult_clear(&bpr);
    return v.status;
}

/* Filters the given event with the given filter and writes the results into a
 * notification */
static UA_StatusCode
UA_Server_filterEvent(UA_Server* server, UA_Session* session,
    const UA_NodeId* eventNode, UA_EventFilter* filter,
    UA_EventNotification* notification) {
    if (filter->selectClausesSize == 0)
        return UA_STATUSCODE_BADEVENTFILTERINVALID;

    UA_EventFieldList_init(&notification->fields);
    /* EventFilterResult isn't being used currently
    UA_EventFilterResult_init(&notification->result); */

    notification->fields.eventFields = (UA_Variant*)
        UA_Array_new(filter->selectClausesSize, &UA_TYPES[UA_TYPES_VARIANT]);
    if (!notification->fields.eventFields) {
        /* EventFilterResult currently isn't being used
        UA_EventFiterResult_clear(&notification->result); */
        return UA_STATUSCODE_BADOUTOFMEMORY;
    }
    notification->fields.eventFieldsSize = filter->selectClausesSize;

    /* EventFilterResult currently isn't being used
    notification->result.selectClauseResultsSize = filter->selectClausesSize;
    notification->result.selectClauseResults = (UA_StatusCode *)
        UA_Array_new(filter->selectClausesSize, &UA_TYPES[UA_TYPES_STATUSCODE]);
    if(!notification->result->selectClauseResults) {
        UA_EventFieldList_clear(&notification->fields);
        UA_EventFilterResult_clear(&notification->result);
        return UA_STATUSCODE_BADOUTOFMEMORY;
    }
    */

    /* Apply the filter */

    /* Check if the browsePath is BaseEventType, in which case nothing more
     * needs to be checked */
    UA_NodeId baseEventTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
    for (size_t i = 0; i < filter->selectClausesSize; i++) {
        if (!UA_NodeId_equal(&filter->selectClauses[i].typeDefinitionId, &baseEventTypeId) &&
            !isValidEvent(server, &filter->selectClauses[i].typeDefinitionId, eventNode)) {
            UA_Variant_init(&notification->fields.eventFields[i]);
            /* EventFilterResult currently isn't being used
            notification->result.selectClauseResults[i] = UA_STATUSCODE_BADTYPEDEFINITIONINVALID; */
            continue;
        }

        /* TODO: Put the result into the selectClausResults */
        resolveSimpleAttributeOperand(server, session, eventNode,
            &filter->selectClauses[i],
            &notification->fields.eventFields[i]);
    }

    return UA_STATUSCODE_GOOD;
}

void QUaBaseEvent::triggerRaw()
{
    if (!this->shouldTrigger())
    {
        return;
    }
    UA_LOCK(m_qUaServer->m_server->serviceMutex);
    UA_StatusCode retval = UA_STATUSCODE_GOOD;

#if UA_LOGLEVEL <= 200
    UA_LOG_NODEID_WRAP(&origin,
        UA_LOG_DEBUG(&server->config.logger, UA_LOGCATEGORY_SERVER,
            "Events: An event is triggered on node %.*s",
            (int)nodeIdStr.length, nodeIdStr.data));
#endif

    // NOTE : do NOT check if condition or branch

    /* Check that the origin node exists */
    const UA_Node* originNode = UA_NODESTORE_GET(m_qUaServer->m_server, &m_sourceNodeId);
    if (!originNode) {
        UA_LOG_ERROR(&m_qUaServer->m_server->config.logger, UA_LOGCATEGORY_USERLAND,
            "Origin node for event does not exist.");
        UA_UNLOCK(m_qUaServer->m_server->serviceMutex);
        Q_ASSERT_X(false, "QUaBaseEvent::triggerInternal", "UA_STATUSCODE_BADNOTFOUND");
        return /*UA_STATUSCODE_BADNOTFOUND*/;
    }
    UA_NODESTORE_RELEASE(m_qUaServer->m_server, originNode);

    /* Make sure the origin is in the ObjectsFolder (TODO: or in the ViewsFolder) */
    if (!isNodeInTree(m_qUaServer->m_server, &m_sourceNodeId, &objectsFolderId,
        emitReferencesRoots, 2)) { /* Only use Organizes and
                                    * HasComponent to check if we
                                    * are below the ObjectsFolder */
        UA_LOG_ERROR(&m_qUaServer->m_server->config.logger, UA_LOGCATEGORY_USERLAND,
            "Node for event must be in ObjectsFolder!");
        UA_UNLOCK(m_qUaServer->m_server->serviceMutex);
        Q_ASSERT_X(false, "QUaBaseEvent::triggerInternal", "UA_STATUSCODE_BADINVALIDARGUMENT");
        return /*UA_STATUSCODE_BADINVALIDARGUMENT*/;
    }

    // NOTE : do NOT set standard fields

    /* List of nodes that emit the node. Events propagate upwards (bubble up) in
     * the node hierarchy. */
    UA_ExpandedNodeId* emitNodes = NULL;
    size_t emitNodesSize = 0;

    /* Add the server node to the list of nodes from which the event is emitted.
     * The server node emits all events.
     *
     * Part 3, 7.17: In particular, the root notifier of a Server, the Server
     * Object defined in Part 5, is always capable of supplying all Events from
     * a Server and as such has implied HasEventSource References to every event
     * source in a Server. */
    UA_NodeId emitStartNodes[2];
    emitStartNodes[0] = m_sourceNodeId;
    emitStartNodes[1] = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);

    /* Get all ReferenceTypes over which the events propagate */
    UA_NodeId* emitRefTypes[EMIT_REFS_ROOT_COUNT] = { NULL, NULL, NULL };
    size_t emitRefTypesSize[EMIT_REFS_ROOT_COUNT] = { 0, 0, 0, 0 };
    size_t totalEmitRefTypesSize = 0;
    for (size_t i = 0; i < EMIT_REFS_ROOT_COUNT; i++) {
        retval |= referenceSubtypes(m_qUaServer->m_server, &emitReferencesRoots[i],
            &emitRefTypesSize[i], &emitRefTypes[i]);
        totalEmitRefTypesSize += emitRefTypesSize[i];
    }
    Q_ASSERT(retval == UA_STATUSCODE_GOOD);
    UA_STACKARRAY(UA_NodeId, totalEmitRefTypes, totalEmitRefTypesSize);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_WARNING(&m_qUaServer->m_server->config.logger, UA_LOGCATEGORY_SERVER,
            "Events: Could not create the list of references for event "
            "propagation with StatusCode %s", UA_StatusCode_name(retval));
        goto cleanup;
    }

    size_t currIndex = 0;
    for (size_t i = 0; i < EMIT_REFS_ROOT_COUNT; i++) {
        memcpy(&totalEmitRefTypes[currIndex], emitRefTypes[i],
            emitRefTypesSize[i] * sizeof(UA_NodeId));
        currIndex += emitRefTypesSize[i];
    }

    /* Get the list of nodes in the hierarchy that emits the event. */
    retval = browseRecursive(m_qUaServer->m_server, 2, emitStartNodes,
        totalEmitRefTypesSize, totalEmitRefTypes,
        UA_BROWSEDIRECTION_INVERSE, true,
        &emitNodesSize, &emitNodes);
    Q_ASSERT(retval == UA_STATUSCODE_GOOD);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_WARNING(&m_qUaServer->m_server->config.logger, UA_LOGCATEGORY_SERVER,
            "Events: Could not create the list of nodes listening on the "
            "event with StatusCode %s", UA_StatusCode_name(retval));
        goto cleanup;
    }

    /* Add the event to the listening MonitoredItems at each relevant node */
    for (size_t i = 0; i < emitNodesSize; i++) {
        const UA_ObjectNode* node = (const UA_ObjectNode*)
            UA_NODESTORE_GET(m_qUaServer->m_server, &emitNodes[i].nodeId);
        if (!node)
            continue;
        if (node->nodeClass != UA_NODECLASS_OBJECT) {
            UA_NODESTORE_RELEASE(m_qUaServer->m_server, (const UA_Node*)node);
            continue;
        }
        for (UA_MonitoredItem* mi = node->monitoredItemQueue; mi != NULL; mi = mi->next) {
            retval = UA_Event_addEventToMonitoredItem(m_qUaServer->m_server, &m_nodeId, mi);
            if (retval != UA_STATUSCODE_GOOD) {
                UA_LOG_WARNING(&m_qUaServer->m_server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Events: Could not add the event to a listening node with StatusCode %s",
                    UA_StatusCode_name(retval));
                retval = UA_STATUSCODE_GOOD; /* Only log problems with individual emit nodes */
            }
        }
        UA_NODESTORE_RELEASE(m_qUaServer->m_server, (const UA_Node*)node);
#ifdef UA_ENABLE_HISTORIZING
        if (!m_qUaServer->m_server->config.historyDatabase.setEvent)
            continue;
        UA_EventFilter* filter = NULL;
        UA_EventFieldList* fieldList = NULL;
        UA_Variant historicalEventFilterValue;
        UA_Variant_init(&historicalEventFilterValue);
        /* a HistoricalEventNode that has event history available will provide this property */
        retval = readObjectProperty(m_qUaServer->m_server, emitNodes[i].nodeId,
            UA_QUALIFIEDNAME(0, (char*)"HistoricalEventFilter"),
            &historicalEventFilterValue);
        Q_ASSERT(retval == UA_STATUSCODE_GOOD);
        /* check if the property was found and the read was successful */
        if (retval != UA_STATUSCODE_GOOD) {
            /* do not vex users with no match errors */
            if (retval != UA_STATUSCODE_BADNOMATCH)
                UA_LOG_WARNING(&m_qUaServer->m_server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Cannot read the HistoricalEventFilter property of a "
                    "listening node. StatusCode %s",
                    UA_StatusCode_name(retval));
        }
        /* if found then check if HistoricalEventFilter property has a valid value */
        else if (UA_Variant_isEmpty(&historicalEventFilterValue) ||
            !UA_Variant_isScalar(&historicalEventFilterValue) ||
            historicalEventFilterValue.type->typeIndex != UA_TYPES_EVENTFILTER) {
            UA_LOG_WARNING(&m_qUaServer->m_server->config.logger, UA_LOGCATEGORY_SERVER,
                "HistoricalEventFilter property of a listening node "
                "does not have a valid value");
        }
        /* finally, if found and valid then filter */
        else {
            filter = (UA_EventFilter*)historicalEventFilterValue.data;
            UA_EventNotification eventNotification;
            retval = UA_Server_filterEvent(m_qUaServer->m_server, &m_qUaServer->m_server->adminSession, &m_nodeId,
                filter, &eventNotification);
            if (retval == UA_STATUSCODE_GOOD) {
                fieldList = UA_EventFieldList_new();
                *fieldList = eventNotification.fields;
            }
            /* eventNotification structure is not cleared so that users can
             * avoid copying the field list if they want to store it */
             /* EventFilterResult isn't being used currently
             UA_EventFilterResult_clear(&notification->result); */
        }
        m_qUaServer->m_server->config.historyDatabase.setEvent(m_qUaServer->m_server, m_qUaServer->m_server->config.historyDatabase.context,
            &m_sourceNodeId, &emitNodes[i].nodeId,
            &m_nodeId, false,
            filter,
            fieldList);
        UA_Variant_clear(&historicalEventFilterValue);
        retval = UA_STATUSCODE_GOOD;
#endif
    }

cleanup:
    for (size_t i = 0; i < EMIT_REFS_ROOT_COUNT; i++) {
        UA_Array_delete(emitRefTypes[i], emitRefTypesSize[i], &UA_TYPES[UA_TYPES_NODEID]);
    }
    UA_Array_delete(emitNodes, emitNodesSize, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]);
    UA_UNLOCK(m_qUaServer->m_server->serviceMutex);
    Q_ASSERT(retval == UA_STATUSCODE_GOOD);
}

bool QUaBaseEvent::shouldTrigger() const
{
    // can trigger only if Qt parent is set
    // i.e. avoid trigger while setting props during deserialization
    return this->parent(); 
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS