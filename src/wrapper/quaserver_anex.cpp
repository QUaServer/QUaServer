#include "quaserver_anex.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaCondition>

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
    if (UA_NodeId_equal(validEventParent, &conditionTypeId) &&
        isNodeInTree_singleRef(server, tEventType, &conditionTypeId,
            UA_REFERENCETYPEINDEX_HASSUBTYPE)) {
        UA_BrowsePathResult_clear(&bpr);
        UA_Variant_clear(&tOutVariant);
        return true;
    }

    /*EventType is not a Subtype of CondtionType
     *(ConditionId Clause won't be present in Events, which are not Conditions)*/
     /* check whether Valid Event other than Conditions */
    UA_NodeId baseEventTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
    UA_Boolean isSubtypeOfBaseEvent =
        isNodeInTree_singleRef(server, tEventType, &baseEventTypeId,
            UA_REFERENCETYPEINDEX_HASSUBTYPE);

    UA_BrowsePathResult_clear(&bpr);
    UA_Variant_clear(&tOutVariant);
    return isSubtypeOfBaseEvent;
}

/* Part 4: 7.4.4.5 SimpleAttributeOperand
 * The clause can point to any attribute of nodes. Either a child of the event
 * node and also the event type. */
UA_StatusCode
QUaServer_Anex::resolveSimpleAttributeOperand(
    UA_Server* server, 
    UA_Session* session, 
    const UA_NodeId* origin,
    const UA_SimpleAttributeOperand* sao, 
    UA_Variant* value,
    const QUaSaoCallback& resolveSAOCallback
) {
    // NOTE : custom code
    if (resolveSAOCallback)
    {
        auto browsePath = QUaQualifiedName::saoToBrowsePath(sao);
        *value = QUaTypesConverter::uaVariantFromQVariant(resolveSAOCallback(browsePath));
        return UA_Variant_isEmpty(value) ? UA_STATUSCODE_BADNOTFOUND : UA_STATUSCODE_GOOD;
    }

    /* Prepare the ReadValueId */
    UA_ReadValueId rvi;
    UA_ReadValueId_init(&rvi);
    rvi.indexRange = sao->indexRange;
    rvi.attributeId = sao->attributeId;

    UA_DataValue v;

    if (sao->browsePathSize == 0) {
        /* If this list (browsePath) is empty, the Node is the instance of the
        * TypeDefinition. */
        rvi.nodeId = sao->typeDefinitionId;

        /* A Condition is an indirection. Look up the target node. */
        /* TODO: check for Branches! One Condition could have multiple Branches */
        UA_NodeId conditionTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE);
        if (UA_NodeId_equal(&sao->typeDefinitionId, &conditionTypeId)) {
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
            /* // NOTE : original code
            UA_StatusCode res = UA_getConditionId(server, origin, &rvi.nodeId);
            if (res != UA_STATUSCODE_GOOD)
                return res;
            */
            // NOTE : custom code
            // get condition object from node context
            void* context;
            auto st = UA_Server_getNodeContext(server, *origin, &context);
            if (st != UA_STATUSCODE_GOOD)
            {
                return UA_STATUSCODE_BADNOTFOUND;
            }
            UA_NodeId conditionId;
            auto condition = qobject_cast<QUaCondition*>(static_cast<QObject*>(context));
            if (condition)
            {
                conditionId = condition->nodeId();
            }
            else
            {
                return UA_STATUSCODE_BADNOTFOUND;
            }
            rvi.nodeId = conditionId;
#else
            return UA_STATUSCODE_BADNOTSUPPORTED;
#endif
        }

        v = UA_Server_readWithSession(server, session, &rvi, UA_TIMESTAMPSTORETURN_NEITHER);

    } else {
        /* Resolve the browse path, starting from the event-source (and not the
         * typeDefinitionId). */
        UA_BrowsePathResult bpr =
            browseSimplifiedBrowsePath(server, *origin, sao->browsePathSize, sao->browsePath);
        if (bpr.targetsSize == 0 && bpr.statusCode == UA_STATUSCODE_GOOD)
            bpr.statusCode = UA_STATUSCODE_BADNOTFOUND;
        if (bpr.statusCode != UA_STATUSCODE_GOOD) {
            UA_StatusCode res = bpr.statusCode;
            UA_BrowsePathResult_clear(&bpr);
            return res;
        }

        /* Use the first match */
        rvi.nodeId = bpr.targets[0].targetId.nodeId;
        v = UA_Server_readWithSession(server, session, &rvi, UA_TIMESTAMPSTORETURN_NEITHER);
        UA_BrowsePathResult_clear(&bpr);
    }

    /* Move the result to the output */
    if (v.status == UA_STATUSCODE_GOOD && v.hasValue)
        *value = v.value;
    else
        UA_Variant_clear(&v.value);
    return v.status;
}

/* Filters the given event with the given filter and writes the results into a
 * notification */
UA_StatusCode
QUaServer_Anex::UA_Server_filterEvent(
    UA_Server* server,
    UA_Session* session,
    const UA_NodeId* eventNode,
    UA_EventFilter* filter,
    UA_EventFieldList* efl,
    const QUaSaoCallback& resolveSAOCallback
) {
    if (filter->selectClausesSize == 0)
        return UA_STATUSCODE_BADEVENTFILTERINVALID;

    UA_StatusCode res =
        UA_Server_evaluateWhereClauseContentFilter(server, eventNode, &filter->whereClause);
    if (res != UA_STATUSCODE_GOOD)
        return res;

    UA_EventFieldList_init(efl);
    efl->eventFields = (UA_Variant*)
        UA_Array_new(filter->selectClausesSize, &UA_TYPES[UA_TYPES_VARIANT]);
    if (!efl->eventFields)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    efl->eventFieldsSize = filter->selectClausesSize;

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
            UA_Variant_init(&efl->eventFields[i]);
            /* EventFilterResult currently isn't being used
            notification->result.selectClauseResults[i] = UA_STATUSCODE_BADTYPEDEFINITIONINVALID; */
            continue;
        }

        /* TODO: Put the result into the selectClausResults */
        QUaServer_Anex::resolveSimpleAttributeOperand(
            server, 
            session, 
            eventNode,
            &filter->selectClauses[i], 
            &efl->eventFields[i],
            resolveSAOCallback
        );
    }

    return UA_STATUSCODE_GOOD;
}

/* Filters an event according to the filter specified by mon and then adds it to
 * mons notification queue */
UA_StatusCode
QUaServer_Anex::UA_Event_addEventToMonitoredItem(
    UA_Server* server, 
    const UA_NodeId* event, 
    UA_MonitoredItem* mon,
    const QUaSaoCallback& resolveSAOCallback
) {
    UA_Notification* notification = UA_Notification_new();
    if (!notification)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    if (mon->parameters.filter.content.decoded.type != &UA_TYPES[UA_TYPES_EVENTFILTER])
        return UA_STATUSCODE_BADFILTERNOTALLOWED;
    UA_EventFilter* eventFilter = (UA_EventFilter*)
        mon->parameters.filter.content.decoded.data;

    UA_Subscription* sub = mon->subscription;
    UA_Session* session = sub->session;
    UA_StatusCode retval = UA_Server_filterEvent(
        server, 
        session, 
        event,
        eventFilter, 
        &notification->data.event,
        resolveSAOCallback
    );
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Notification_delete(server, notification);
        if (retval == UA_STATUSCODE_BADNOMATCH)
            return UA_STATUSCODE_GOOD;
        return retval;
    }

    notification->data.event.clientHandle = mon->parameters.clientHandle;
    notification->mon = mon;

    // [FIX] : when updating to 1.2, the remove condition UA_MonitoredItem_ensureQueueSpace started
    //         killing events if in a cycle there was more than 1 event. Seems to be a bug introduced by
    //         moving maxQueueSize from UA_MonitoredItem into the  sub struct UA_MonitoringParameters
    // https://github.com/open62541/open62541/commit/e95a98a542ea94b7a2f19f5019f659f11c857d2c#r46617160
    mon->parameters.queueSize = -1;

    UA_Notification_enqueueAndTrigger(server, notification);
    return UA_STATUSCODE_GOOD;
}

static const UA_NodeId objectsFolderId = {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_OBJECTSFOLDER}};
#define EMIT_REFS_ROOT_COUNT 4
static const UA_NodeId emitReferencesRoots[EMIT_REFS_ROOT_COUNT] =
    {{0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_ORGANIZES}},
     {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASCOMPONENT}},
     {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASEVENTSOURCE}},
     {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASNOTIFIER}}};

static const UA_NodeId isInFolderReferences[2] =
    {{0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_ORGANIZES}},
     {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASCOMPONENT}}};

UA_StatusCode
QUaServer_Anex::UA_Server_triggerEvent_Modified(
    UA_Server* server, 
    const UA_NodeId eventNodeId,
    const UA_NodeId origin,
    const QUaSaoCallback& resolveSAOCallback/* = nullptr*/
) {
    UA_LOCK(server->serviceMutex);

    UA_LOG_NODEID_DEBUG(&origin,
        UA_LOG_DEBUG(&server->config.logger, UA_LOGCATEGORY_SERVER,
            "Events: An event is triggered on node %.*s",
            (int)nodeIdStr.length, nodeIdStr.data));

    // [MODIFIED] : do not check if condition or branch

    /* Check that the origin node exists */
    const UA_Node* originNode = UA_NODESTORE_GET(server, &origin);
    if (!originNode) {
        UA_LOG_ERROR(&server->config.logger, UA_LOGCATEGORY_USERLAND,
            "Origin node for event does not exist.");
        UA_UNLOCK(server->serviceMutex);
        return UA_STATUSCODE_BADNOTFOUND;
    }
    UA_NODESTORE_RELEASE(server, originNode);

    /* Make sure the origin is in the ObjectsFolder (TODO: or in the ViewsFolder) */
    /* Only use Organizes and HasComponent to check if we are below the ObjectsFolder */
    UA_StatusCode retval;
    UA_ReferenceTypeSet refTypes;
    UA_ReferenceTypeSet_init(&refTypes);
    for (int i = 0; i < 2; ++i) {
        UA_ReferenceTypeSet tmpRefTypes;
        retval = referenceTypeIndices(server, &isInFolderReferences[i], &tmpRefTypes, true);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_WARNING(&server->config.logger, UA_LOGCATEGORY_SERVER,
                "Events: Could not create the list of references and their subtypes "
                "with StatusCode %s", UA_StatusCode_name(retval));
        }
        refTypes = UA_ReferenceTypeSet_union(refTypes, tmpRefTypes);
    }

    if (!isNodeInTree(server, &origin, &objectsFolderId, &refTypes)) {
        UA_LOG_ERROR(&server->config.logger, UA_LOGCATEGORY_USERLAND,
            "Node for event must be in ObjectsFolder!");
        UA_UNLOCK(server->serviceMutex);
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    // [MODIFIED] : do not update the standard fields

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
    emitStartNodes[0] = origin;
    emitStartNodes[1] = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);

    // [MODIFIED] : custom history code
 #ifdef UA_ENABLE_HISTORIZING
    // get event instance
    auto event = qobject_cast<QUaBaseEvent*>(QUaNode::getNodeContext(eventNodeId, server));
    Q_ASSERT(event);
    bool historize = event->historizing();
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
    if (resolveSAOCallback)
    {
        auto condition = qobject_cast<QUaCondition*>(event);
        Q_ASSERT(condition);
        historize = historize && condition->historizingBranches();
    }
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
    QList<QUaNodeId> emittersNodeIds;
    auto srv = QUaServer::getServerNodeContext(server);
    Q_ASSERT(srv);
    if (historize && srv->eventHistoryRead())
    {
        emittersNodeIds << UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);
    }
#endif // UA_ENABLE_HISTORIZING

    /* Get all ReferenceTypes over which the events propagate */
    UA_ReferenceTypeSet emitRefTypes;
    UA_ReferenceTypeSet_init(&emitRefTypes);
    for (size_t i = 0; i < EMIT_REFS_ROOT_COUNT; i++) {
        UA_ReferenceTypeSet tmpRefTypes;
        retval = referenceTypeIndices(server, &emitReferencesRoots[i], &tmpRefTypes, true);
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_WARNING(&server->config.logger, UA_LOGCATEGORY_SERVER,
                "Events: Could not create the list of references for event "
                "propagation with StatusCode %s", UA_StatusCode_name(retval));
            goto cleanup;
        }
        emitRefTypes = UA_ReferenceTypeSet_union(emitRefTypes, tmpRefTypes);
    }

    /* Get the list of nodes in the hierarchy that emits the event. */
    retval = browseRecursive(server, 2, emitStartNodes, UA_BROWSEDIRECTION_INVERSE,
        &emitRefTypes, UA_NODECLASS_UNSPECIFIED, true,
        &emitNodesSize, &emitNodes);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_WARNING(&server->config.logger, UA_LOGCATEGORY_SERVER,
            "Events: Could not create the list of nodes listening on the "
            "event with StatusCode %s", UA_StatusCode_name(retval));
        goto cleanup;
    }

    /* Add the event to the listening MonitoredItems at each relevant node */
    for (size_t i = 0; i < emitNodesSize; i++) 
    {
        /* Get the node */
        const UA_ObjectNode* node = (const UA_ObjectNode*)
            UA_NODESTORE_GET(server, &emitNodes[i].nodeId);
        if (!node)
            continue;

        /* Only consider objects */
        if (node->head.nodeClass != UA_NODECLASS_OBJECT) {
            UA_NODESTORE_RELEASE(server, (const UA_Node*)node);
            continue;
        }

        /* Add event to monitoreditems */
        for (UA_MonitoredItem* mi = node->monitoredItemQueue; mi != NULL; mi = mi->next) {
            retval = UA_Event_addEventToMonitoredItem(server, &eventNodeId, mi, resolveSAOCallback);
            if (retval != UA_STATUSCODE_GOOD) {
                UA_LOG_WARNING(&server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Events: Could not add the event to a listening node with StatusCode %s",
                    UA_StatusCode_name(retval));
                retval = UA_STATUSCODE_GOOD; /* Only log problems with individual emit nodes */
            }
        }

        UA_NODESTORE_RELEASE(server, (const UA_Node*)node);

#ifdef UA_ENABLE_HISTORIZING
        // [MODIFIED] : custom history code
        if (historize)
        {
            //do not support filter "HistoricalEventFilter" 
            // NOTE : delete a bunch of stuff of the original history plugin
            // get emitter instance
            auto emitter = qobject_cast<QUaBaseObject*>(QUaNode::getNodeContext(emitNodes[i].nodeId, server));
            // NOTE : emitter can be null for default objects (open62541?) like UA_NS0ID_ROOTFOLDER
            if (emitter && emitter->eventHistoryRead())
            {
                emittersNodeIds << emitNodes[i].nodeId;
            }
        }
#endif // UA_ENABLE_HISTORIZING
    }
    // [MODIFIED] : custom history code
#ifdef UA_ENABLE_HISTORIZING
    if (historize && emittersNodeIds.count() > 0)
    {
        // get event type node id
        QUaNodeId eventTypeNodeId = event->typeDefinitionNodeId();
        Q_ASSERT(srv->m_hashTypeVars.contains(eventTypeNodeId));
        // populate history point
        QUaHistoryEventPoint eventPoint;
        auto& typeData = srv->m_hashTypeVars[eventTypeNodeId];
        auto i = typeData.begin();
        while (i != typeData.end())
        {
            auto &name = i.key();
            ++i;
            QVariant value;
            if (resolveSAOCallback)
            {
                value = resolveSAOCallback(
                    QUaBrowsePath() << name
                );
            }
            else
            {
                auto var = event->browsePath<QUaBaseVariable>(name);
                value = var ? var->value() : QVariant();
            }
            // NOTE : do not if (!value.isValid()), else branchId column will not be created
            eventPoint.fields[name] = value;
        }
        const static auto eventNodeIdPath      = QUaBrowsePath() << QUaQualifiedName(0, "EventNodeId");
        const static auto originatorNodeIdPath = QUaBrowsePath() << QUaQualifiedName(0, "OriginNodeId");
        Q_ASSERT(!eventPoint.fields.contains(eventNodeIdPath));
        Q_ASSERT(!eventPoint.fields.contains(originatorNodeIdPath));
        // add event node id and origin node id
        eventPoint.fields[eventNodeIdPath] = QVariant::fromValue(QUaNodeId(eventNodeId));
        eventPoint.fields[originatorNodeIdPath] = QVariant::fromValue(QUaNodeId(origin));
        // add timestamp
        eventPoint.timestamp = event->time();
        // store
        /*bool ok = */QUaHistoryBackend::setEvent(
            srv,
            eventTypeNodeId,
            emittersNodeIds,
            eventPoint
        );
        //NOTE : can fail due to historizer not set, which is acceptable Q_ASSERT(ok);
    }
#endif // UA_ENABLE_HISTORIZING

    // [MODIFIED] : do not delete node

cleanup:
    UA_Array_delete(emitNodes, emitNodesSize, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]);
    UA_UNLOCK(server->serviceMutex);
    return retval;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
