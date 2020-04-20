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
    this->setSeverity(0);
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

QUaNodeId QUaBaseEvent::eventType() const
{
	return const_cast<QUaBaseEvent*>(this)->getEventType()->value<QUaNodeId>();
}

void QUaBaseEvent::setEventType(const QUaNodeId& eventTypeNodeId)
{
	return this->getEventType()->setValue(eventTypeNodeId);
}

QUaNodeId QUaBaseEvent::sourceNode() const
{
	return const_cast<QUaBaseEvent*>(this)->getSourceNode()->value<QUaNodeId>();
}

void QUaBaseEvent::setSourceNode(const QUaNodeId& sourceNodeId)
{
	// set cache
	UA_NodeId_clear(&m_sourceNodeId);
	m_sourceNodeId = sourceNodeId;
    // get node
    QUaNode* node = m_qUaServer->nodeById(sourceNodeId);
    QUaBaseObject* obj = qobject_cast<QUaBaseObject*>(node);
    if (node && !obj)
    {
        Q_ASSERT_X(false, "QUaBaseEvent::setSourceNode", "Source node must be a object");
        return;
    }
    if (obj)
    {
        // source node must be an event notifier
        obj->setEventNotifier(true);
        // there is supposed to be a (non-looping) event generation hierarchy
        // TODO : implement event hierarchy according to Part 3 - 7.x
        // for now all source nodes are directly event sources of server object directly
        auto st = UA_Server_addReference(
            m_qUaServer->m_server,
            UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER),
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASEVENTSOURCE /*UA_NS0ID_HASNOTIFIER*/),
            { obj->m_nodeId, UA_STRING_NULL, 0 },
            true
        );
        Q_ASSERT(st == UA_STATUSCODE_GOOD || st == UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED);
    }
    Q_ASSERT(
          obj ||
        (!obj && sourceNodeId == UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)) ||
        (!obj && sourceNodeId.isNull())
    );
	// set internally
	return this->getSourceNode()->setValue(sourceNodeId);
}

void QUaBaseEvent::setSourceNode(const QUaNode* sourceNode)
{
    this->setSourceNode(sourceNode ? sourceNode->nodeId()      : QUaNodeId());
    this->setSourceName(sourceNode ? sourceNode->displayName() : "");
}

QString QUaBaseEvent::sourceName() const
{
	return const_cast<QUaBaseEvent*>(this)->getSourceName()->value<QString>();
}

void QUaBaseEvent::setSourceName(const QString & strSourceName)
{
	this->getSourceName()->setValue(strSourceName);
}

QDateTime QUaBaseEvent::time() const
{
	return const_cast<QUaBaseEvent*>(this)->getTime()->value<QDateTime>().toUTC();
}

void QUaBaseEvent::setTime(const QDateTime & dateTime)
{
	this->getTime()->setValue(dateTime.toUTC());
}

QDateTime QUaBaseEvent::receiveTime() const
{
	return const_cast<QUaBaseEvent*>(this)->getReceiveTime()->value<QDateTime>().toUTC();
}

void QUaBaseEvent::setReceiveTime(const QDateTime& dateTime)
{
    this->getReceiveTime()->setValue(dateTime.toUTC());
}

// NOTE : removed because is optional and open62541 now does not add it
QTimeZone QUaBaseEvent::localTime() const
{
	return const_cast<QUaBaseEvent*>(this)->getLocalTime()->value<QTimeZone>();
}

void QUaBaseEvent::setLocalTime(const QTimeZone & localTimeZone)
{
	this->getLocalTime()->setValue(localTimeZone);
}

QUaLocalizedText QUaBaseEvent::message() const
{
	return const_cast<QUaBaseEvent*>(this)->getMessage()->value<QUaLocalizedText>();
}

void QUaBaseEvent::setMessage(const QUaLocalizedText& message)
{
	this->getMessage()->setValue(message);
}

quint16 QUaBaseEvent::severity() const
{
	return (quint16)const_cast<QUaBaseEvent*>(this)->getSeverity()->value<quint16>();
}

void QUaBaseEvent::setSeverity(const quint16 & intSeverity)
{
	this->getSeverity()->setValue(intSeverity);
}

void QUaBaseEvent::trigger()
{
    this->setEventId(QUaBaseEvent::generateEventId());
    this->triggerInternal();
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
static const UA_NodeId emitReferencesRoots[EMIT_REFS_ROOT_COUNT] ={ 
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_ORGANIZES     }},
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASCOMPONENT  }},
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASEVENTSOURCE}},
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASNOTIFIER   }} 
};

void QUaBaseEvent::triggerInternal()
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
     /* Set the SourceNode (origin) */
     /* Set the ReceiveTime */
     /* Set the EventId */

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
            retval = UA_Event_addEventToMonitoredItem(
                m_qUaServer->m_server, 
                &m_nodeId, 
                mi
            );
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

    /* DO NOT Delete the node representation of the event */

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