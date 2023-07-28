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

/*
 * Singly-linked List definitions.
 */
#define SLIST_HEAD(name, type)						\
struct name {								\
    struct type *slh_first;	/* first element */			\
}

#define	SLIST_HEAD_INITIALIZER(head)					\
    { NULL }

#define SLIST_ENTRY(type)						\
struct {								\
    struct type *sle_next;	/* next element */			\
}

typedef enum {
    UA_SERVERLIFECYCLE_FRESH,
    UA_SERVERLIFECYLE_RUNNING
} UA_ServerLifecycle;

struct ContinuationPoint;

typedef struct UA_SessionHeader {
    SLIST_ENTRY(UA_SessionHeader) next;
    UA_NodeId authenticationToken;
#if UA_OPEN62541_VER_MINOR > 2
    UA_Boolean serverSession; /* Disambiguate client and server session */
#endif
    UA_SecureChannel* channel; /* The pointer back to the SecureChannel in the session. */
} UA_SessionHeader;

typedef struct {
    UA_SessionHeader  header;
    UA_ApplicationDescription clientDescription;
    UA_String         sessionName;
    UA_Boolean        activated;
    void*             sessionHandle; /* pointer assigned in userland-callback */
    UA_NodeId         sessionId;
    UA_UInt32         maxRequestMessageSize;
    UA_UInt32         maxResponseMessageSize;
    UA_Double         timeout; /* in ms */
    UA_DateTime       validTill;
    UA_ByteString     serverNonce;
    UA_UInt16         availableContinuationPoints;
    ContinuationPoint* continuationPoints;

#if UA_OPEN62541_VER_MINOR > 2
    size_t paramsSize;
    UA_KeyValuePair *params;

    /* Localization information */
    size_t localeIdsSize;
    UA_String *localeIds;
#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /* The queue is ordered according to the priority byte (higher bytes come
     * first). When a late subscription finally publishes, then it is pushed to
     * the back within the sub-set of subscriptions that has the same priority
     * (round-robin scheduling). */
    size_t subscriptionsSize;
    TAILQ_HEAD(, UA_Subscription) subscriptions;

#if UA_OPEN62541_VER_MINOR > 2
    size_t responseQueueSize;
#endif

    SIMPLEQ_HEAD(, UA_PublishResponseEntry) responseQueue;

#if UA_OPEN62541_VER_MINOR < 3
    UA_UInt32 numPublishReq;
#endif

    size_t totalRetransmissionQueueSize; /* Retransmissions of all subscriptions */
#endif

#if UA_OPEN62541_VER_MINOR > 2
#ifdef UA_ENABLE_DIAGNOSTICS
    UA_SessionSecurityDiagnosticsDataType securityDiagnostics;
    UA_SessionDiagnosticsDataType diagnostics;
#endif // UA_ENABLE_DIAGNOSTICS
#endif // UA_OPEN62541_VER_MINOR
} UA_Session;

extern "C" UA_Session *
UA_Server_getSessionById(UA_Server * server, const UA_NodeId * sessionId);

typedef void (*UA_ApplicationCallback)(void* application, void* data);

typedef struct UA_TimerEntry {
#if UA_OPEN62541_VER_MINOR < 3
    ZIP_ENTRY(UA_TimerEntry) zipfields;
    UA_DateTime nextTime;                    /* The next time when the callback
                                              * is to be executed */
    UA_UInt64 interval;                      /* Interval in 100ns resolution. If
                                                the interval is zero, the
                                                callback is not repeated and
                                                removed after execution. */
    UA_ApplicationCallback callback;
    void* application;
    void* data;

    ZIP_ENTRY(UA_TimerEntry) idZipfields;
#else
    struct aa_entry treeEntry;
    UA_TimerPolicy timerPolicy;              /* Timer policy to handle cycle misses */
    UA_DateTime nextTime;                    /* The next time when the callback
                                              * is to be executed */
    UA_UInt64 interval;                      /* Interval in 100ns resolution. If
                                                the interval is zero, the
                                                callback is not repeated and
                                                removed after execution. */
    UA_ApplicationCallback callback;
    void* application;
    void* data;

    struct aa_entry idTreeEntry;
#endif
    UA_UInt64 id;                            /* Id of the entry */
} UA_TimerEntry;

typedef struct session_list_entry {
    UA_TimerEntry cleanupCallback;
    LIST_ENTRY(session_list_entry) pointers;
    UA_Session session;
} session_list_entry;

struct UA_TimerEntry;
typedef struct UA_TimerEntry UA_TimerEntry;

ZIP_HEAD(UA_TimerZip, UA_TimerEntry);
typedef struct UA_TimerZip UA_TimerZip;

ZIP_HEAD(UA_TimerIdZip, UA_TimerEntry);
typedef struct UA_TimerIdZip UA_TimerIdZip;

typedef struct {
#if UA_OPEN62541_VER_MINOR < 3
    UA_TimerZip root;     /* The root of the time-sorted zip tree */
    UA_TimerIdZip idRoot; /* The root of the id-sorted zip tree */
    UA_UInt64 idCounter;  /* Generate unique identifiers. Identifiers are always
                           * above zero. */
#if UA_MULTITHREADING >= 100
    UA_LOCK_TYPE(timerMutex)
#endif
#else
    struct aa_head root;   /* The root of the time-sorted tree */
    struct aa_head idRoot; /* The root of the id-sorted tree */
    UA_UInt64 idCounter;   /* Generate unique identifiers. Identifiers are
                            * always above zero. */
#if UA_MULTITHREADING >= 100
    UA_Lock timerMutex;
#endif
#endif // UA_OPEN62541_VER_MINOR
} UA_Timer;

// NOTE : just gave it a name in order to be able to use
//        TAILQ_LAST in QUaServer::newSession
TAILQ_HEAD(channel_list, channel_entry);

typedef TAILQ_HEAD(UA_MessageQueue, UA_Message) UA_MessageQueue;

/**
 * MessageType
 * ^^^^^^^^^^^
 * Message Type and whether the message contains an intermediate chunk */
typedef enum {
    UA_MESSAGETYPE_ACK = 0x4B4341,
    UA_MESSAGETYPE_HEL = 0x4C4548,
    UA_MESSAGETYPE_MSG = 0x47534D,
    UA_MESSAGETYPE_OPN = 0x4E504F,
    UA_MESSAGETYPE_CLO = 0x4F4C43,
    UA_MESSAGETYPE_ERR = 0x525245,
    UA_MESSAGETYPE_INVALID = 0x0,
    __UA_MESSAGETYPE_FORCE32BIT = 0x7fffffff
} UA_MessageType;
UA_STATIC_ASSERT(sizeof(UA_MessageType) == sizeof(UA_Int32), enum_must_be_32bit);

/**
 * ChunkType
 * ^^^^^^^^^
 * Type of the chunk */
typedef enum {
    UA_CHUNKTYPE_FINAL = 0x46000000,
    UA_CHUNKTYPE_INTERMEDIATE = 0x43000000,
    UA_CHUNKTYPE_ABORT = 0x41000000,
    __UA_CHUNKTYPE_FORCE32BIT = 0x7fffffff
} UA_ChunkType;
UA_STATIC_ASSERT(sizeof(UA_ChunkType) == sizeof(UA_Int32), enum_must_be_32bit);

/* For chunked requests */
typedef struct UA_Chunk {
    SIMPLEQ_ENTRY(UA_Chunk) pointers;
    UA_ByteString bytes;
    UA_MessageType messageType;
    UA_ChunkType chunkType;
    UA_UInt32 requestId;
    UA_Boolean copied; /* Do the bytes point to a buffer from the network or was
                        * memory allocated for the chunk separately */
} UA_Chunk;

/**
 * AsymmetricAlgorithmSecurityHeader
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * Asymmetric Security Header */
typedef struct {
    UA_ByteString securityPolicyUri;
    UA_ByteString senderCertificate;
    UA_ByteString receiverCertificateThumbprint;
} UA_AsymmetricAlgorithmSecurityHeader;

typedef SIMPLEQ_HEAD(UA_ChunkQueue, UA_Chunk) UA_ChunkQueue;

typedef enum {
    UA_SECURECHANNELRENEWSTATE_NORMAL,

    /* Client has sent an OPN, but not received a response so far. */
    UA_SECURECHANNELRENEWSTATE_SENT,

    /* The server waits for the first request with the new token for the rollover.
     * The new token is stored in the altSecurityToken. The configured local and
     * remote symmetric encryption keys are the old ones. */
     UA_SECURECHANNELRENEWSTATE_NEWTOKEN_SERVER,

     /* The client already uses the new token. But he waits for the server to respond
      * with the new token to complete the rollover. The old token is stored in
      * altSecurityToken. The local symmetric encryption key is new. The remote
      * encryption key is the old one. */
      UA_SECURECHANNELRENEWSTATE_NEWTOKEN_CLIENT
} UA_SecureChannelRenewState;

struct UA_SecureChannel {
    UA_SecureChannelState state;
    UA_SecureChannelRenewState renewState;
    UA_MessageSecurityMode securityMode;
    UA_ConnectionConfig config;

    /* Rules for revolving the token with a renew OPN request: The client is
     * allowed to accept messages with the old token until the OPN response has
     * arrived. The server accepts the old token until one message secured with
     * the new token has arrived.
     *
     * We recognize whether nextSecurityToken contains a valid next token if the
     * ChannelId is not 0. */
    UA_ChannelSecurityToken securityToken;    /* Also contains the channelId */
    UA_ChannelSecurityToken altSecurityToken; /* Alternative token for the rollover.
                                               * See the renewState. */

                                               /* The endpoint and context of the channel */
    const UA_SecurityPolicy* securityPolicy;
    void* channelContext; /* For interaction with the security policy */
    UA_Connection* connection;

    /* Asymmetric encryption info */
    UA_ByteString remoteCertificate;
    UA_Byte remoteCertificateThumbprint[20]; /* The thumbprint of the remote certificate */

    /* Symmetric encryption nonces. These are used to generate the key material
     * and must not be reused once the keys are in place.
     *
     * Nonces are also used during the CreateSession / ActivateSession
     * handshake. These are not handled here, as the Session handling can
     * overlap with a RenewSecureChannel. */
    UA_ByteString remoteNonce;
    UA_ByteString localNonce;

    UA_UInt32 receiveSequenceNumber;
    UA_UInt32 sendSequenceNumber;

    /* Sessions that are bound to the SecureChannel */
    SLIST_HEAD(, UA_SessionHeader) sessions;

    /* If a buffer is received, first all chunks are put into the completeChunks
     * queue. Then they are processed in order. This ensures that processing
     * buffers is reentrant with the correct processing order. (This has lead to
     * problems in the client in the past.) */
    UA_ChunkQueue completeChunks; /* Received full chunks that have not been
                                   * decrypted so far */
    UA_ChunkQueue decryptedChunks; /* Received chunks that were decrypted but
                                    * not processed */
    size_t decryptedChunksCount;
    size_t decryptedChunksLength;
    UA_ByteString incompleteChunk; /* A half-received chunk (TCP is a
                                    * streaming protocol) is stored here */

    UA_CertificateVerification* certificateVerification;
    UA_StatusCode(*processOPNHeader)(void* application, UA_SecureChannel* channel,
        const UA_AsymmetricAlgorithmSecurityHeader* asymHeader);
};

typedef struct channel_entry {
    UA_TimerEntry cleanupCallback;
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
#if UA_OPEN62541_VER_MINOR > 2
    UA_UInt32 activeSessionCount;
#endif
    UA_Session adminSession; /* Local access to the services (for startup and
                              * maintenance) uses this Session with all possible
                              * access rights (Session Id: 1) */

                              /* Namespaces */
    size_t namespacesSize;
    UA_String* namespaces;

    /* Callbacks with a repetition interval */
    UA_Timer timer;

    /* For bootstrapping, omit some consistency checks, creating a reference to
     * the parent and member instantiation */
    UA_Boolean bootstrapNS0;

    /* Discovery */
#ifdef UA_ENABLE_DISCOVERY
    UA_DiscoveryManager discoveryManager;
#endif

    /* Subscriptions */
#ifdef UA_ENABLE_SUBSCRIPTIONS
    size_t subscriptionsSize;  /* Number of active subscriptions */
    size_t monitoredItemsSize; /* Number of active monitored items */
    LIST_HEAD(, UA_Subscription) subscriptions; /* All subscriptions in the
                                                 * server. They may be detached
                                                 * from a session. */
    UA_UInt32 lastSubscriptionId; /* To generate unique SubscriptionIds */

    /* To be cast to UA_LocalMonitoredItem to get the callback and context */
    LIST_HEAD(, UA_MonitoredItem) localMonitoredItems;
    UA_UInt32 lastLocalMonitoredItemId;

# ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#if UA_OPEN62541_VER_MINOR < 3
    LIST_HEAD(, UA_ConditionSource) headConditionSource;
#else
    LIST_HEAD(, UA_ConditionSource) conditionSources;
#endif
# endif

#endif

    /* Publish/Subscribe */
#ifdef UA_ENABLE_PUBSUB
    UA_PubSubManager pubSubManager;
#endif

#if UA_MULTITHREADING >= 100
#if UA_OPEN62541_VER_MINOR < 3
    UA_LOCK_TYPE(networkMutex)
    UA_LOCK_TYPE(serviceMutex)
#else
    UA_Lock networkMutex;
    UA_Lock serviceMutex;
#endif
#endif

        /* Statistics */
#if UA_OPEN62541_VER_MINOR < 3
    UA_ServerStatistics serverStats;
#else
    UA_NetworkStatistics networkStatistics;
    UA_SecureChannelStatistics secureChannelStatistics;
    UA_ServerDiagnosticsSummaryDataType serverDiagnosticsSummary;
#endif
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

typedef void
(*UA_TimerExecutionCallback)(void* executionApplication, UA_ApplicationCallback cb,
    void* callbackApplication, void* data);

extern "C"
void UA_Server_cleanupSessions(UA_Server* server, UA_DateTime nowMonotonic);

extern "C"
void UA_Server_removeSession(UA_Server * server, session_list_entry * sentry, UA_DiagnosticEvent event);

extern "C"
void UA_Server_deleteSecureChannels(UA_Server* server);

extern "C"
void removeSecureChannel(UA_Server * server, channel_entry * entry, UA_DiagnosticEvent event);

extern "C"
void UA_Server_cleanupTimedOutSecureChannels(UA_Server* server, UA_DateTime nowMonotonic);

extern "C"
void UA_Discovery_cleanupTimedOut(UA_Server * server, UA_DateTime nowMonotonic);

extern "C"
UA_DateTime
UA_Timer_process(UA_Timer * t, UA_DateTime nowMonotonic,
    UA_TimerExecutionCallback executionCallback,
    void* executionApplication);

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
#if UA_OPEN62541_VER_MINOR > 2
    UA_CertificateVerification verifyX509;
#endif
} AccessControlContext;

extern "C" const UA_String anonymous_policy;
extern "C" const UA_String username_policy;
#if UA_OPEN62541_VER_MINOR > 2
extern "C" const UA_String certificate_policy;
#endif

#ifdef UA_ENABLE_HISTORIZING
/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaServer::getGathering
*/
typedef struct {
    UA_HistoryDataGathering gathering;
} UA_HistoryDatabaseContext_default;

typedef struct {
    UA_NodeId nodeId;
    UA_HistorizingNodeIdSettings setting;
    UA_MonitoredItemCreateResult monitoredResult;
} UA_NodeIdStoreContextItem_gathering_default;

typedef struct {
    UA_NodeIdStoreContextItem_gathering_default* dataStore;
    size_t storeEnd;
    size_t storeSize;
} UA_NodeIdStoreContext;

inline static UA_NodeIdStoreContextItem_gathering_default*
getNodeIdStoreContextItem_gathering_default(UA_NodeIdStoreContext* context,
    const UA_NodeId* nodeId)
{
    for (size_t i = 0; i < context->storeEnd; ++i) {
        if (UA_NodeId_equal(&context->dataStore[i].nodeId, nodeId)) {
            return &context->dataStore[i];
        }
    }
    return NULL;
}
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
getNodeType(UA_Server * server, const UA_NodeHead * head);

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

typedef TAILQ_HEAD(NotificationQueue, UA_Notification) NotificationQueue;
#if UA_OPEN62541_VER_MINOR > 2
typedef TAILQ_HEAD(NotificationMessageQueue, UA_NotificationMessageEntry)
    NotificationMessageQueue;

/* The type of sampling for MonitoredItems depends on the sampling interval.
 *
 * >0: Cyclic callback
 * =0: Attached to the node. Sampling is triggered after every "write".
 * <0: Attached to the subscription. Triggered just before every "publish". */
typedef enum {
    UA_MONITOREDITEMSAMPLINGTYPE_NONE = 0,
    UA_MONITOREDITEMSAMPLINGTYPE_CYCLIC, /* Cyclic callback */
    UA_MONITOREDITEMSAMPLINGTYPE_EVENT,  /* Attached to the node. Can be a "write
                                          * event" for DataChange MonitoredItems
                                          * with a zero sampling interval .*/
    UA_MONITOREDITEMSAMPLINGTYPE_PUBLISH /* Attached to the subscription */
} UA_MonitoredItemSamplingType;
#endif

struct UA_MonitoredItem {
    UA_TimerEntry delayedFreePointers;
    LIST_ENTRY(UA_MonitoredItem) listEntry;
#if UA_OPEN62541_VER_MINOR < 3
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    UA_MonitoredItem* next; /* Linked list of MonitoredItems directly attached
                             * to a Node */
#endif
#endif // UA_OPEN62541_VER_MINOR
    UA_Subscription* subscription; /* If NULL, then this is a Local MonitoredItem */
    UA_UInt32 monitoredItemId;

    /* Status and Settings */
    UA_ReadValueId itemToMonitor;
    UA_MonitoringMode monitoringMode;
    UA_TimestampsToReturn timestampsToReturn;
#if UA_OPEN62541_VER_MINOR < 3
    UA_Boolean sampleCallbackIsRegistered;
#endif
    UA_Boolean registered; /* Registered in the server / Subscription */
#if UA_OPEN62541_VER_MINOR > 2
    UA_DateTime triggeredUntil;  /* If the MonitoringMode is SAMPLING,
                                  * triggering the MonitoredItem puts the latest
                                  * Notification into the publishing queue (of
                                  * the Subscription). In addition, the first
                                  * new sample is also published (and not just
                                  * sampled) if it occurs within the duration of
                                  * one publishing cycle after the triggering. */
#endif
    /* If the filter is a UA_DataChangeFilter: The DataChangeFilter always
     * contains an absolute deadband definition. Part 8, §6.2 gives the
     * following formula to test for percentage deadbands:
     *
     * DataChange if (absolute value of (last cached value - current value)
     *                > (deadbandValue/100.0) * ((high–low) of EURange)))
     *
     * So we can convert from a percentage to an absolute deadband and keep
     * the hot code path simple.
     *
     * TODO: Store the percentage deadband to recompute when the UARange is
     * changed at runtime of the MonitoredItem */
    UA_MonitoringParameters parameters;

    /* Sampling Callback */
#if UA_OPEN62541_VER_MINOR < 3
    UA_UInt64 sampleCallbackId;
    UA_ByteString lastSampledValue;
#else
    UA_MonitoredItemSamplingType samplingType;
    union {
        UA_UInt64 callbackId;
        UA_MonitoredItem *nodeListNext; /* Event-Based: Attached to Node */
        LIST_ENTRY(UA_MonitoredItem) samplingListEntry; /* Publish-interval: Linked in
                                                         * Subscription */
    } sampling;
#endif
    UA_DataValue lastValue;

    /* Triggering Links */
    size_t triggeringLinksSize;
    UA_UInt32* triggeringLinks;

    /* Notification Queue */
    NotificationQueue queue;
    size_t queueSize; /* This is the current size. See also the configured
                       * (maximum) queueSize in the parameters. */
    size_t eventOverflows; /* Separate counter for the queue. Can at most double
                            * the queue size */
};

typedef struct UA_Notification {
#if UA_OPEN62541_VER_MINOR < 3
    TAILQ_ENTRY(UA_Notification) listEntry;   /* Notification list for the MonitoredItem */
#else
    TAILQ_ENTRY(UA_Notification) localEntry;  /* Notification list for the MonitoredItem */
#endif
    TAILQ_ENTRY(UA_Notification) globalEntry; /* Notification list for the Subscription */

    UA_MonitoredItem* mon; /* Always set */
    /* The event field is used if mon->attributeId is the EventNotifier */
    union {
        UA_MonitoredItemNotification dataChange;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
        UA_EventFieldList event;
#endif
    } data;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    UA_Boolean isOverflowEvent; /* Counted manually */
#if UA_OPEN62541_VER_MINOR > 2
    UA_EventFilterResult result;
#endif // UA_OPEN62541_VER_MINOR
#endif
} UA_Notification;

extern "C"
UA_StatusCode
UA_Event_generateEventId(UA_ByteString * generatedId);

extern "C"
UA_StatusCode
browseRecursive(UA_Server * server, size_t startNodesSize, const UA_NodeId * startNodes,
    UA_BrowseDirection browseDirection, const UA_ReferenceTypeSet * refTypes,
    UA_UInt32 nodeClassMask, UA_Boolean includeStartNodes,
    size_t * resultsSize, UA_ExpandedNodeId * *results);

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
isNodeInTree(UA_Server * server, const UA_NodeId * leafNode,
    const UA_NodeId * nodeToFind, const UA_ReferenceTypeSet * relevantRefs);

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
    UA_TimerEntry delayedFreePointers;
    LIST_ENTRY(UA_Subscription) serverListEntry;
    /* Ordered according to the priority byte and round-robin scheduling for
     * late subscriptions. See ua_session.h. Only set if session != NULL. */
    TAILQ_ENTRY(UA_Subscription) sessionListEntry;
    UA_Session* session; /* May be NULL if no session is attached. */
    UA_UInt32 subscriptionId;

    /* Settings */
    UA_UInt32 lifeTimeCount;
    UA_UInt32 maxKeepAliveCount;
    UA_Double publishingInterval; /* in ms */
    UA_UInt32 notificationsPerPublish;
    UA_Boolean publishingEnabled;
#if UA_OPEN62541_VER_MINOR < 3
    UA_UInt32 priority;
#else
    UA_Byte priority;
#endif

    /* Runtime information */
    UA_SubscriptionState state;
    UA_StatusCode statusChange; /* If set, a notification is generated and the
                                 * Subscription is deleted within
                                 * UA_Subscription_publish. */
    UA_UInt32 nextSequenceNumber;
    UA_UInt32 currentKeepAliveCount;
    UA_UInt32 currentLifetimeCount;

    /* Publish Callback. Registered if id > 0. */
    UA_UInt64 publishCallbackId;

    /* MonitoredItems */
    UA_UInt32 lastMonitoredItemId; /* increase the identifiers */
    LIST_HEAD(, UA_MonitoredItem) monitoredItems;
    UA_UInt32 monitoredItemsSize;

#if UA_OPEN62541_VER_MINOR > 2
    /* MonitoredItems that are sampled in every publish callback (with the
     * publish interval of the subscription) */
    LIST_HEAD(, UA_MonitoredItem) samplingMonitoredItems;
#endif

    /* Global list of notifications from the MonitoredItems */
#if UA_OPEN62541_VER_MINOR < 3
    NotificationQueue notificationQueue;
#else
    TAILQ_HEAD(, UA_Notification) notificationQueue;
#endif
    UA_UInt32 notificationQueueSize; /* Total queue size */
    UA_UInt32 dataChangeNotifications;
    UA_UInt32 eventNotifications;

#if UA_OPEN62541_VER_MINOR < 3
    /* Notifications to be sent out now (already late). In a regular publish
     * callback, all queued notifications are sent out. In a late publish
     * response, only the notifications left from the last regular publish
     * callback are sent. */
    UA_UInt32 readyNotifications;
#endif

    /* Retransmission Queue */
#if UA_OPEN62541_VER_MINOR < 3
    ListOfNotificationMessages retransmissionQueue;
#else
    NotificationMessageQueue retransmissionQueue;
#endif
    size_t retransmissionQueueSize;

#if UA_OPEN62541_VER_MINOR > 2
    /* Statistics for the server diagnostics. The fields are defined according
     * to the SubscriptionDiagnosticsDataType (Part 5, §12.15). */
#ifdef UA_ENABLE_DIAGNOSTICS
    UA_UInt32 modifyCount;
    UA_UInt32 enableCount;
    UA_UInt32 disableCount;
    UA_UInt32 republishRequestCount;
    UA_UInt32 republishMessageCount;
    UA_UInt32 transferRequestCount;
    UA_UInt32 transferredToAltClientCount;
    UA_UInt32 transferredToSameClientCount;
    UA_UInt32 publishRequestCount;
    UA_UInt32 dataChangeNotificationsCount;
    UA_UInt32 eventNotificationsCount;
    UA_UInt32 notificationsCount;
    UA_UInt32 latePublishRequestCount;
    UA_UInt32 discardedMessageCount;
    UA_UInt32 monitoringQueueOverflowCount;
    UA_UInt32 eventQueueOverFlowCount;
#endif // UA_ENABLE_DIAGNOSTICS
#endif // UA_OPEN62541_VER_MINOR
};

extern "C"
UA_MonitoredItem *
UA_Subscription_getMonitoredItem(UA_Subscription * sub, UA_UInt32 monitoredItemId);

extern "C"
UA_StatusCode
UA_Server_evaluateWhereClauseContentFilter(
    UA_Server * server,
#if UA_OPEN62541_VER_MINOR < 3
    const UA_NodeId * eventNode,
    const UA_ContentFilter * contentFilter);
#else
    UA_Session *session,
    const UA_NodeId * eventNode,
    const UA_ContentFilter * contentFilter,
    UA_ContentFilterResult * contentFilterResult);
#endif

extern "C"
UA_Notification *
UA_Notification_new(void);

extern "C"
#if UA_OPEN62541_VER_MINOR < 3
void UA_Notification_delete(UA_Server* server, UA_Notification* n);
#else
void UA_Notification_delete(UA_Notification *n);
#endif

extern "C"
void
UA_Notification_enqueueAndTrigger(UA_Server* server, UA_Notification* n);

extern "C"
UA_StatusCode
referenceTypeIndices(UA_Server* server, const UA_NodeId* refType,
    UA_ReferenceTypeSet* indices, UA_Boolean includeSubtypes);

extern "C"
UA_Boolean
isNodeInTree_singleRef(UA_Server * server, const UA_NodeId * leafNode,
    const UA_NodeId * nodeToFind, const UA_Byte relevantRefTypeIndex);

class QUaServer_Anex
{
    friend class QUaServer;
    friend class QUaBaseEvent;
#ifdef UA_ENABLE_HISTORIZING
    friend class QUaHistoryBackend;
#endif // UA_ENABLE_HISTORIZING
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
    friend class QUaCondition;
    friend class QUaConditionBranch;
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

    typedef std::function<QVariant(const QUaBrowsePath&)> QUaSaoCallback;

    static UA_StatusCode resolveSimpleAttributeOperand(
        UA_Server* server, 
        UA_Session* session, 
        const UA_NodeId* origin,
        const UA_SimpleAttributeOperand* sao, 
        UA_Variant* value,
        const QUaSaoCallback& resolveSAOCallback
    );

    static UA_StatusCode UA_Server_filterEvent(
        UA_Server* server,
        UA_Session* session,
        const UA_NodeId* eventNode,
        UA_EventFilter* filter,
        UA_EventFieldList* efl,
#if UA_OPEN62541_VER_MINOR > 2
        UA_EventFilterResult *result,
#endif
        const QUaSaoCallback& resolveSAOCallback
    );

    static UA_StatusCode UA_Event_addEventToMonitoredItem(
        UA_Server* server, 
        const UA_NodeId* event, 
        UA_MonitoredItem* mon,
        const QUaSaoCallback& resolveSAOCallback
    );

    static UA_StatusCode UA_Server_triggerEvent_Modified(
        UA_Server* server,
        const UA_NodeId eventNodeId,
        const UA_NodeId origin,
        const QUaSaoCallback& resolveSAOCallback = nullptr
    );
};

/* Print a NodeId in logs */
#define UA_LOG_NODEID_WRAP(LEVEL, NODEID, LOG)       \
    if(UA_LOGLEVEL <= LEVEL) {                       \
        UA_String nodeIdStr = UA_STRING_NULL;        \
        UA_NodeId_print(NODEID, &nodeIdStr);         \
        LOG;                                         \
        UA_String_clear(&nodeIdStr);                 \
    }

#if UA_LOGLEVEL <= 200
# define UA_LOG_NODEID_DEBUG(NODEID, LOG)       \
    UA_LOG_NODEID_INTERNAL(NODEID, LOG)
#else
# define UA_LOG_NODEID_DEBUG(NODEID, LOG)
#endif

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
