#include "quaserver.h"

/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaServer::stop
call UA_SecureChannelManager_deleteMembers
call UA_SessionManager_deleteMembers
*/

/*
 * List definitions.
 */
#define LIST_HEAD(name, type)						\
struct name {								\
    struct type *lh_first;	/* first element */			\
}

#define LIST_HEAD_INITIALIZER(head)					\
    { NULL }

#define LIST_ENTRY(type)						\
struct {								\
    struct type *le_next;	/* next element */			\
    struct type **le_prev;	/* address of previous next element */	\
}

 /*
  * List access methods
  */
#define	LIST_FIRST(head)		((head)->lh_first)
#define	LIST_END(head)			NULL
#define	LIST_EMPTY(head)		(LIST_FIRST(head) == LIST_END(head))
#define	LIST_NEXT(elm, field)		((elm)->field.le_next)

#define LIST_FOREACH(var, head, field)					\
    for((var) = LIST_FIRST(head);					\
        (var)!= LIST_END(head);					\
        (var) = LIST_NEXT(var, field))

#define	LIST_FOREACH_SAFE(var, head, field, tvar)			\
    for ((var) = LIST_FIRST(head);				\
        (var) && ((tvar) = LIST_NEXT(var, field), 1);		\
        (var) = (tvar))

 /*
  * Simple queue definitions.
  */
#define SIMPLEQ_HEAD(name, type)					\
struct name {								\
    struct type *sqh_first;	/* first element */			\
    struct type **sqh_last;	/* addr of last next element */		\
}

#define SIMPLEQ_HEAD_INITIALIZER(head)					\
    { NULL, &(head).sqh_first }

#define SIMPLEQ_ENTRY(type)						\
struct {								\
    struct type *sqe_next;	/* next element */			\
}

  /*
   * Tail queue definitions.
   */
#define TAILQ_HEAD(name, type)						\
struct name {								\
    struct type *tqh_first;	/* first element */			\
    struct type **tqh_last;	/* addr of last next element */		\
}

#define TAILQ_HEAD_INITIALIZER(head)					\
    { NULL, &(head).tqh_first }

#define TAILQ_ENTRY(type)						\
struct {								\
    struct type *tqe_next;	/* next element */			\
    struct type **tqe_prev;	/* address of previous next element */	\
}

#define ZIP_HEAD(name, type)                    \
struct name {                                   \
    struct type *zip_root;                      \
}

   /*
    * tail queue access methods
    */
#define	TAILQ_FIRST(head)		((head)->tqh_first)
#define	TAILQ_END(head)			NULL
#define	TAILQ_NEXT(elm, field)		((elm)->field.tqe_next)
#define TAILQ_LAST(head, headname)					\
    (*(((struct headname *)((head)->tqh_last))->tqh_last))
    /* XXX */
#define TAILQ_PREV(elm, headname, field)				\
    (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#define	TAILQ_EMPTY(head)						\
    (TAILQ_FIRST(head) == TAILQ_END(head))

#define TAILQ_FOREACH(var, head, field)					\
    for((var) = TAILQ_FIRST(head);					\
        (var) != TAILQ_END(head);					\
        (var) = TAILQ_NEXT(var, field))

#define	TAILQ_FOREACH_SAFE(var, head, field, tvar)			\
    for ((var) = TAILQ_FIRST(head);					\
        (var) != TAILQ_END(head) &&					\
        ((tvar) = TAILQ_NEXT(var, field), 1);			\
        (var) = (tvar))


#define TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
    for((var) = TAILQ_LAST(head, headname);				\
        (var) != TAILQ_END(head);					\
        (var) = TAILQ_PREV(var, headname, field))

#define	TAILQ_FOREACH_REVERSE_SAFE(var, head, headname, field, tvar)	\
    for ((var) = TAILQ_LAST(head, headname);			\
        (var) != TAILQ_END(head) &&					\
        ((tvar) = TAILQ_PREV(var, headname, field), 1);		\
        (var) = (tvar))

typedef enum {
    UA_SERVERLIFECYCLE_FRESH,
    UA_SERVERLIFECYLE_RUNNING
} UA_ServerLifecycle;

struct ContinuationPoint;

typedef struct UA_SessionHeader {
    UA_NodeId authenticationToken;
    UA_SecureChannel* channel; /* The pointer back to the SecureChannel in the session. */
} UA_SessionHeader;

typedef struct {
    UA_SessionHeader  header;
    UA_ApplicationDescription clientDescription;
    UA_String         sessionName;
    UA_Boolean        activated;
    void* sessionHandle; // pointer assigned in userland-callback
    UA_NodeId         sessionId;
    UA_UInt32         maxRequestMessageSize;
    UA_UInt32         maxResponseMessageSize;
    UA_Double         timeout; // [ms]
    UA_DateTime       validTill;
    UA_ByteString     serverNonce;
    UA_UInt16 availableContinuationPoints;
    ContinuationPoint* continuationPoints;
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_UInt32 lastSubscriptionId;
    UA_UInt32 lastSeenSubscriptionId;
    LIST_HEAD(UA_ListOfUASubscriptions, UA_Subscription) serverSubscriptions;
    SIMPLEQ_HEAD(UA_ListOfQueuedPublishResponses, UA_PublishResponseEntry) responseQueue;
    UA_UInt32        numSubscriptions;
    UA_UInt32        numPublishReq;
    size_t           totalRetransmissionQueueSize; /* Retransmissions of all subscriptions */
#endif
} UA_Session;

extern "C" UA_Session *
UA_Server_getSessionById(UA_Server * server, const UA_NodeId * sessionId);

typedef void (*UA_ApplicationCallback)(void* application, void* data);

typedef struct UA_DelayedCallback {
    SIMPLEQ_ENTRY(UA_DelayedCallback) next;
    UA_ApplicationCallback callback;
    void* application;
    void* data;
} UA_DelayedCallback;

typedef struct session_list_entry {
    UA_DelayedCallback cleanupCallback;
    LIST_ENTRY(session_list_entry) pointers;
    UA_Session session;
} session_list_entry;

struct UA_TimerEntry;
typedef struct UA_TimerEntry UA_TimerEntry;

ZIP_HEAD(UA_TimerZip, UA_TimerEntry);
typedef struct UA_TimerZip UA_TimerZip;

ZIP_HEAD(UA_TimerIdZip, UA_TimerEntry);
typedef struct UA_TimerIdZip UA_TimerIdZip;

/* Only for a single thread. Protect by a mutex if required. */
typedef struct {
    UA_TimerZip root; /* The root of the time-sorted zip tree */
    UA_TimerIdZip idRoot; /* The root of the id-sorted zip tree */
    UA_UInt64 idCounter;
} UA_Timer;

struct UA_WorkQueue {
    /* Worker threads and work queue. Without multithreading, work is executed
       immediately. */
#if UA_MULTITHREADING >= 200
    UA_Worker* workers;
    size_t workersSize;

    /* Work queue */
    SIMPLEQ_HEAD(, UA_DelayedCallback) dispatchQueue; /* Dispatch queue for the worker threads */
    UA_LOCK_TYPE(dispatchQueue_accessMutex) /* mutex for access to queue */
        pthread_cond_t dispatchQueue_condition; /* so the workers don't spin if the queue is empty */
    UA_LOCK_TYPE(dispatchQueue_conditionMutex) /* mutex for access to condition variable */
#endif

    /* Delayed callbacks
     * To be executed after all curretly dispatched works has finished */
        SIMPLEQ_HEAD(, UA_DelayedCallback) delayedCallbacks;
#if UA_MULTITHREADING >= 200
    UA_LOCK_TYPE(delayedCallbacks_accessMutex)
        UA_DelayedCallback* delayedCallbacks_checkpoint;
    size_t delayedCallbacks_sinceDispatch; /* How many have been added since we
                                            * tried to dispatch callbacks? */
#endif
};

typedef struct {
    LIST_HEAD(, periodicServerRegisterCallback_entry) periodicServerRegisterCallbacks;
    LIST_HEAD(, registeredServer_list_entry) registeredServers;
    size_t registeredServersSize;
    UA_Server_registerServerCallback registerServerCallback;
    void* registerServerCallbackData;

# ifdef UA_ENABLE_DISCOVERY_MULTICAST
    mdns_daemon_t* mdnsDaemon;
    UA_SOCKET mdnsSocket;
    UA_Boolean mdnsMainSrvAdded;

    /* Full Domain Name of server itself. Used to detect if received mDNS message was from itself */
    UA_String selfFqdnMdnsRecord;

    LIST_HEAD(, serverOnNetwork_list_entry) serverOnNetwork;

    UA_UInt32 serverOnNetworkRecordIdCounter;
    UA_DateTime serverOnNetworkRecordIdLastReset;

    /* hash mapping domain name to serverOnNetwork list entry */
    struct serverOnNetwork_hash_entry* serverOnNetworkHash[SERVER_ON_NETWORK_HASH_SIZE];

    UA_Server_serverOnNetworkCallback serverOnNetworkCallback;
    void* serverOnNetworkCallbackData;

#if UA_MULTITHREADING >= 200
    pthread_t mdnsThread;
    UA_Boolean mdnsRunning;
#  endif
# endif /* UA_ENABLE_DISCOVERY_MULTICAST */
} UA_DiscoveryManager;

// NOTE : just gave it a name in order to be able to use
//        TAILQ_LAST in QUaServer::newSession
TAILQ_HEAD(channel_list, channel_entry);

typedef enum {
    UA_SECURECHANNELSTATE_FRESH,
    UA_SECURECHANNELSTATE_HEL_SENT,
    UA_SECURECHANNELSTATE_HEL_RECEIVED,
    UA_SECURECHANNELSTATE_ACK_SENT,
    UA_SECURECHANNELSTATE_ACK_RECEIVED,
    UA_SECURECHANNELSTATE_OPN_SENT,
    UA_SECURECHANNELSTATE_OPEN,
    UA_SECURECHANNELSTATE_CLOSED
} UA_SecureChannelState;

typedef TAILQ_HEAD(UA_MessageQueue, UA_Message) UA_MessageQueue;

struct UA_SecureChannel {
    UA_SecureChannelState   state;
    UA_MessageSecurityMode  securityMode;
    UA_ConnectionConfig     config;

    /* Rules for revolving the token with a renew OPN request: The client is
     * allowed to accept messages with the old token until the OPN response has
     * arrived. The server accepts the old token until one message secured with
     * the new token has arrived.
     *
     * We recognize whether nextSecurityToken contains a valid next token if the
     * ChannelId is not 0. */
    UA_ChannelSecurityToken securityToken;     /* Also contains the channelId */
    UA_ChannelSecurityToken nextSecurityToken; /* Only used by the server. The next token
                                                * is put here when sending the OPN
                                                * response. */

                                                /* The endpoint and context of the channel */
    const UA_SecurityPolicy* securityPolicy;
    void* channelContext; /* For interaction with the security policy */
    UA_Connection* connection;

    /* Asymmetric encryption info */
    UA_ByteString remoteCertificate;
    UA_Byte remoteCertificateThumbprint[20]; /* The thumbprint of the remote certificate */

    /* Symmetric encryption info */
    UA_ByteString remoteNonce;
    UA_ByteString localNonce;

    UA_UInt32 receiveSequenceNumber;
    UA_UInt32 sendSequenceNumber;

    /* The standard does not forbid a SecureChannel to carry several Sessions.
     * But this is not supported here. So clients need one SecureChannel for
     * every Session. */
    UA_SessionHeader* session;

    UA_ByteString incompleteChunk; /* A half-received chunk (TCP is a
                                    * streaming protocol) is stored here */
    UA_MessageQueue messages;      /* Received full chunks grouped into the
                                    * messages */
    UA_Boolean retryReceived;      /* Processing of received chunks was stopped
                                    * e.g. after an OPN message. Retry
                                    * processing remaining chunks. */
};

typedef struct channel_entry {
    UA_DelayedCallback cleanupCallback;
    TAILQ_ENTRY(channel_entry) pointers;
    UA_SecureChannel channel;
} channel_entry;

struct UA_Server {
    /* Config */
    UA_ServerConfig config;
    UA_DateTime startTime;
    UA_DateTime endTime; /* Zeroed out. If a time is set, then the server shuts
                          * down once the time has been reached */

    UA_ServerLifecycle state;

    /* SecureChannels */
    TAILQ_HEAD(, channel_entry) channels;
    UA_UInt32 lastChannelId;
    UA_UInt32 lastTokenId;

#if UA_MULTITHREADING >= 100
    UA_AsyncManager asyncManager;
#endif

    /* Session Management */
    LIST_HEAD(session_list, session_list_entry) sessions;
    UA_UInt32 sessionCount;
    UA_Session adminSession; /* Local access to the services (for startup and
                              * maintenance) uses this Session with all possible
                              * access rights (Session Id: 1) */

                              /* Namespaces */
    size_t namespacesSize;
    UA_String* namespaces;

    /* Callbacks with a repetition interval */
    UA_Timer timer;

    /* WorkQueue and worker threads */
    UA_WorkQueue workQueue;

    /* For bootstrapping, omit some consistency checks, creating a reference to
     * the parent and member instantiation */
    UA_Boolean bootstrapNS0;

    /* Discovery */
#ifdef UA_ENABLE_DISCOVERY
    UA_DiscoveryManager discoveryManager;
#endif

    /* DataChange Subscriptions */
#ifdef UA_ENABLE_SUBSCRIPTIONS
    /* Num active subscriptions */
    UA_UInt32 numSubscriptions;
    /* Num active monitored items */
    UA_UInt32 numMonitoredItems;
    /* To be cast to UA_LocalMonitoredItem to get the callback and context */
    LIST_HEAD(LocalMonitoredItems, UA_MonitoredItem) localMonitoredItems;
    UA_UInt32 lastLocalMonitoredItemId;

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
    LIST_HEAD(conditionSourcelisthead, UA_ConditionSource) headConditionSource;
#endif//UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif

    /* Publish/Subscribe */
#ifdef UA_ENABLE_PUBSUB
    UA_PubSubManager pubSubManager;
#endif

#if UA_MULTITHREADING >= 100
    UA_LOCK_TYPE(networkMutex)
        UA_LOCK_TYPE(serviceMutex)
#endif

        /* Statistics */
        UA_ServerStatistics serverStats;
};

/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaServer::stop
*/

typedef enum {
    UA_DIAGNOSTICEVENT_CLOSE,
    UA_DIAGNOSTICEVENT_REJECT,
    UA_DIAGNOSTICEVENT_SECURITYREJECT,
    UA_DIAGNOSTICEVENT_TIMEOUT,
    UA_DIAGNOSTICEVENT_ABORT,
    UA_DIAGNOSTICEVENT_PURGE
} UA_DiagnosticEvent;

extern "C"
void UA_Server_cleanupSessions(UA_Server* server, UA_DateTime nowMonotonic);

extern "C"
void UA_Server_removeSession(UA_Server * server, session_list_entry * sentry, UA_DiagnosticEvent event);

extern "C"
void UA_Server_deleteSecureChannels(UA_Server* server);

extern "C"
void removeSecureChannel(UA_Server * server, channel_entry * entry, UA_DiagnosticEvent event);

/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaServer::anonymousLoginAllowed
QUaServer::setAnonymousLoginAllowed
set AccessControlContext::allowAnonymous
*/

typedef struct {
    UA_Boolean allowAnonymous;
    size_t usernamePasswordLoginSize;
    UA_UsernamePasswordLogin* usernamePasswordLogin;
} AccessControlContext;

#define ANONYMOUS_POLICY "open62541-anonymous-policy"
#define USERNAME_POLICY "open62541-username-policy"
const UA_String anonymous_policy = UA_STRING_STATIC(ANONYMOUS_POLICY);
const UA_String username_policy = UA_STRING_STATIC(USERNAME_POLICY);


#ifdef UA_ENABLE_HISTORIZING
/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaServer::getGathering
*/
typedef struct {
    UA_HistoryDataGathering gathering;
} UA_HistoryDatabaseContext_default;
#endif // UA_ENABLE_HISTORIZING

/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaNode::instantiateOptionalChild
*/

/***************************/
/* Nodestore Access Macros */
/***************************/

#define UA_NODESTORE_NEW(server, nodeClass)                             \
    server->config.nodestore.newNode(server->config.nodestore.context, nodeClass)

#define UA_NODESTORE_DELETE(server, node)                               \
    server->config.nodestore.deleteNode(server->config.nodestore.context, node)

#define UA_NODESTORE_GET(server, nodeid)                                \
    server->config.nodestore.getNode(server->config.nodestore.context, nodeid)

#define UA_NODESTORE_RELEASE(server, node)                              \
    server->config.nodestore.releaseNode(server->config.nodestore.context, node)

#define UA_NODESTORE_GETCOPY(server, nodeid, outnode)                      \
    server->config.nodestore.getNodeCopy(server->config.nodestore.context, \
                                         nodeid, outnode)

#define UA_NODESTORE_INSERT(server, node, addedNodeId)                    \
    server->config.nodestore.insertNode(server->config.nodestore.context, \
                                        node, addedNodeId)

#define UA_NODESTORE_REPLACE(server, node)                              \
    server->config.nodestore.replaceNode(server->config.nodestore.context, node)

#define UA_NODESTORE_REMOVE(server, nodeId)                             \
    server->config.nodestore.removeNode(server->config.nodestore.context, nodeId)

#define CONDITION_ASSERT_RETURN_RETVAL(retval, logMessage, deleteFunction)                \
    {                                                                                     \
        if(retval != UA_STATUSCODE_GOOD) {                                                \
            UA_LOG_ERROR(&server->config.logger, UA_LOGCATEGORY_USERLAND,                  \
                         logMessage". StatusCode %s", UA_StatusCode_name(retval));        \
            deleteFunction                                                                \
            return retval;                                                                \
        }                                                                                 \
    }

#define CONDITION_ASSERT_RETURN_VOID(retval, logMessage, deleteFunction)                  \
    {                                                                                     \
        if(retval != UA_STATUSCODE_GOOD) {                                                \
            UA_LOG_ERROR(&server->config.logger, UA_LOGCATEGORY_USERLAND,                  \
                         logMessage". StatusCode %s", UA_StatusCode_name(retval));        \
            deleteFunction                                                                \
            return;                                                                       \
        }                                                                                 \
    }

extern "C"
const UA_Node *
getNodeType(UA_Server * server, const UA_Node * node);

/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaServer::registerTypeDefaults
*/

extern "C"
UA_StatusCode
setMethodNode_callback(UA_Server * server,
    const UA_NodeId methodNodeId,
    UA_MethodCallback methodCallback);




#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

typedef struct UA_EventNotification {
    UA_EventFieldList fields;
    /* EventFilterResult currently isn't being used
    UA_EventFilterResult result; */
} UA_EventNotification;

typedef TAILQ_HEAD(NotificationQueue, UA_Notification) NotificationQueue;

struct UA_MonitoredItem {
    UA_DelayedCallback delayedFreePointers;
    LIST_ENTRY(UA_MonitoredItem) listEntry;
    UA_Subscription* subscription; /* Local MonitoredItem if the subscription is NULL */
    UA_UInt32 monitoredItemId;
    UA_UInt32 clientHandle;
    UA_Boolean registered; /* Was the MonitoredItem registered in Userland with
                              the callback? */

                              /* Settings */
    UA_TimestampsToReturn timestampsToReturn;
    UA_MonitoringMode monitoringMode;
    UA_NodeId monitoredNodeId;
    UA_UInt32 attributeId;
    UA_String indexRange;
    UA_Double samplingInterval; // [ms]
    UA_Boolean discardOldest;
    union {
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
        /* If attributeId == UA_ATTRIBUTEID_EVENTNOTIFIER */
        UA_EventFilter eventFilter;
#endif
        /* The DataChangeFilter always contains an absolute deadband definition.
         * Part 8, §6.2 gives the following formula to test for percentage
         * deadbands:
         *
         * DataChange if (absolute value of (last cached value - current value)
         *                > (deadbandValue/100.0) * ((high–low) of EURange)))
         *
         * So we can convert from a percentage to an absolute deadband and keep
         * the hot code path simple.
         *
         * TODO: Store the percentage deadband to recompute when the UARange is
         * changed at runtime of the MonitoredItem */
        UA_DataChangeFilter dataChangeFilter;
    } filter;
    UA_Variant lastValue; // TODO: dataEncoding is hardcoded to UA binary

    /* Sample Callback */
    UA_UInt64 sampleCallbackId;
    UA_ByteString lastSampledValue;
    UA_Boolean sampleCallbackIsRegistered;

    /* Notification Queue */
    NotificationQueue queue;
    UA_UInt32 maxQueueSize; /* The max number of enqueued notifications (not
                             * counting overflow events) */
    UA_UInt32 queueSize;
    UA_UInt32 eventOverflows; /* Separate counter for the queue. Can at most
                               * double the queue size */

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    UA_MonitoredItem* next;
#endif

#ifdef UA_ENABLE_DA
    UA_StatusCode lastStatus;
#endif
};

extern "C"
UA_StatusCode
UA_Event_generateEventId(UA_ByteString * generatedId);

extern "C"
UA_StatusCode
referenceSubtypes(UA_Server* server, const UA_NodeId* refType,
    size_t* refTypesSize, UA_NodeId** refTypes);

extern "C"
UA_StatusCode
browseRecursive(UA_Server* server,
    size_t startNodesSize, const UA_NodeId* startNodes,
    size_t refTypesSize, const UA_NodeId* refTypes,
    UA_BrowseDirection browseDirection, UA_Boolean includeStartNodes,
    size_t* resultsSize, UA_ExpandedNodeId** results);

extern "C"
UA_StatusCode
UA_Event_addEventToMonitoredItem(UA_Server * server, const UA_NodeId * event, UA_MonitoredItem * mon);

extern "C"
UA_StatusCode
readObjectProperty(UA_Server * server, const UA_NodeId objectId,
    const UA_QualifiedName propertyName,
    UA_Variant * value);

extern "C"
UA_StatusCode
UA_getConditionId(UA_Server * server, const UA_NodeId * conditionNodeId,
    UA_NodeId * outConditionId);

extern "C"
UA_DataValue
UA_Server_readWithSession(UA_Server * server, UA_Session * session,
    const UA_ReadValueId * item,
    UA_TimestampsToReturn timestampsToReturn);;

extern "C"
UA_BrowsePathResult
browseSimplifiedBrowsePath(UA_Server * server, const UA_NodeId origin,
    size_t browsePathSize, const UA_QualifiedName * browsePath);

extern "C"
UA_StatusCode
readWithReadValue(UA_Server * server, const UA_NodeId * nodeId,
    const UA_AttributeId attributeId, void* v);

extern "C"
UA_Boolean
isNodeInTree(UA_Server * server, const UA_NodeId * leafNode, const UA_NodeId * nodeToFind,
    const UA_NodeId * referenceTypeIds, size_t referenceTypeIdsSize);

extern "C"
UA_Subscription *
UA_Session_getSubscriptionById(UA_Session * session, UA_UInt32 subscriptionId);

/* We use only a subset of the states defined in the standard */
typedef enum {
    /* UA_SUBSCRIPTIONSTATE_CLOSED */
    /* UA_SUBSCRIPTIONSTATE_CREATING */
    UA_SUBSCRIPTIONSTATE_NORMAL,
    UA_SUBSCRIPTIONSTATE_LATE,
    UA_SUBSCRIPTIONSTATE_KEEPALIVE
} UA_SubscriptionState;

typedef TAILQ_HEAD(ListOfNotificationMessages, UA_NotificationMessageEntry) ListOfNotificationMessages;

struct UA_Subscription {
    UA_DelayedCallback delayedFreePointers;
    LIST_ENTRY(UA_Subscription) listEntry;
    UA_Session* session;
    UA_UInt32 subscriptionId;

    /* Settings */
    UA_UInt32 lifeTimeCount;
    UA_UInt32 maxKeepAliveCount;
    UA_Double publishingInterval; /* in ms */
    UA_UInt32 notificationsPerPublish;
    UA_Boolean publishingEnabled;
    UA_UInt32 priority;

    /* Runtime information */
    UA_SubscriptionState state;
    UA_UInt32 nextSequenceNumber;
    UA_UInt32 currentKeepAliveCount;
    UA_UInt32 currentLifetimeCount;

    /* Publish Callback */
    UA_UInt64 publishCallbackId;
    UA_Boolean publishCallbackIsRegistered;

    /* MonitoredItems */
    UA_UInt32 lastMonitoredItemId; /* increase the identifiers */
    LIST_HEAD(, UA_MonitoredItem) monitoredItems;
    UA_UInt32 monitoredItemsSize;

    /* Global list of notifications from the MonitoredItems */
    NotificationQueue notificationQueue;
    UA_UInt32 notificationQueueSize; /* Total queue size */
    UA_UInt32 dataChangeNotifications;
    UA_UInt32 eventNotifications;
    UA_UInt32 statusChangeNotifications;

    /* Notifications to be sent out now (already late). In a regular publish
     * callback, all queued notifications are sent out. In a late publish
     * response, only the notifications left from the last regular publish
     * callback are sent. */
    UA_UInt32 readyNotifications;

    /* Retransmission Queue */
    ListOfNotificationMessages retransmissionQueue;
    size_t retransmissionQueueSize;
};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS










#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS