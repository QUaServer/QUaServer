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
        UA_NodeId conditionTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE);

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
        //TODO check for Branches! One Condition could have multiple Branches
        // Set ConditionId
        if (UA_NodeId_equal(&sao->typeDefinitionId, &conditionTypeId)) {
            //UA_NodeId conditionId;
            //UA_StatusCode retval = UA_getConditionId(server, origin, &conditionId);
            //if (retval != UA_STATUSCODE_GOOD)
            //    return retval;

            // get condition object from node context
            void* context;
            auto st = UA_Server_getNodeContext(server, *origin, &context);
            if (st != UA_STATUSCODE_GOOD)
            {
                return UA_STATUSCODE_BADNOTFOUND;
            }
            auto condition = qobject_cast<QUaCondition*>(static_cast<QObject*>(context));
            if (!condition)
            {
                return UA_STATUSCODE_BADNOTFOUND;
            }
            UA_NodeId conditionId;
            if (condition->isBranch())
            {
                auto mainBranch = condition->mainBranch();
                Q_ASSERT(mainBranch);
                if (!mainBranch)
                {
                    return UA_STATUSCODE_BADNOTFOUND;
                }
                conditionId = mainBranch->nodeId();
            }
            else
            {
                conditionId = condition->nodeId();
            }
            rvi.nodeId = conditionId;
        }
        else
            rvi.nodeId = sao->typeDefinitionId;
#else
        if (UA_NodeId_equal(&sao->typeDefinitionId, &conditionTypeId))
            return UA_STATUSCODE_BADNOTSUPPORTED;
        else
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
UA_StatusCode
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

/* Filters an event according to the filter specified by mon and then adds it to
 * mons notification queue */
UA_StatusCode
UA_Event_addEventToMonitoredItem(UA_Server* server, const UA_NodeId* event, UA_MonitoredItem* mon) {
    UA_Notification* notification = (UA_Notification*)UA_malloc(sizeof(UA_Notification));
    if (!notification)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    /* Get the session */
    UA_Subscription* sub = mon->subscription;
    UA_Session* session = sub->session;


    /* Apply the filter */
    UA_StatusCode retval =
        UA_Server_filterEvent(server, session, event, &mon->filter.eventFilter,
            &notification->data.event);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_free(notification);
        return retval;
    }

    /* Enqueue the notification */
    notification->mon = mon;
    UA_Notification_enqueue(server, mon->subscription, mon, notification);
    return UA_STATUSCODE_GOOD;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS