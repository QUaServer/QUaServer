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

typedef struct UA_SessionManager {
    LIST_HEAD(session_list, session_list_entry) sessions; // doubly-linked list of sessions
    UA_UInt32 currentSessionCount;
    UA_Server* server;
} UA_SessionManager;

struct ContinuationPoint;

typedef struct UA_SessionHeader {
    LIST_ENTRY(UA_SessionHeader) pointers;
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

typedef struct {
    UA_TimerZip root; /* The root of the time-sorted zip tree */
    UA_TimerIdZip idRoot; /* The root of the id-sorted zip tree */
    UA_UInt64 idCounter;
} UA_Timer;

struct UA_WorkQueue {
    /* Worker threads and work queue. Without multithreading, work is executed
       immediately. */
#ifdef UA_ENABLE_MULTITHREADING
    UA_Worker* workers;
    size_t workersSize;

    /* Work queue */
    SIMPLEQ_HEAD(, UA_DelayedCallback) dispatchQueue; /* Dispatch queue for the worker threads */
    pthread_mutex_t dispatchQueue_accessMutex; /* mutex for access to queue */
    pthread_cond_t dispatchQueue_condition; /* so the workers don't spin if the queue is empty */
    pthread_mutex_t dispatchQueue_conditionMutex; /* mutex for access to condition variable */
#endif

    /* Delayed callbacks
     * To be executed after all curretly dispatched works has finished */
    SIMPLEQ_HEAD(, UA_DelayedCallback) delayedCallbacks;
#ifdef UA_ENABLE_MULTITHREADING
    pthread_mutex_t delayedCallbacks_accessMutex;
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

    LIST_HEAD(, serverOnNetwork_list_entry) serverOnNetwork;
    size_t serverOnNetworkSize;

    UA_UInt32 serverOnNetworkRecordIdCounter;
    UA_DateTime serverOnNetworkRecordIdLastReset;

    /* hash mapping domain name to serverOnNetwork list entry */
    struct serverOnNetwork_hash_entry* serverOnNetworkHash[SERVER_ON_NETWORK_HASH_PRIME];

    UA_Server_serverOnNetworkCallback serverOnNetworkCallback;
    void* serverOnNetworkCallbackData;

#  ifdef UA_ENABLE_MULTITHREADING
    pthread_t mdnsThread;
    UA_Boolean mdnsRunning;
#  endif
# endif /* UA_ENABLE_DISCOVERY_MULTICAST */
} UA_DiscoveryManager;

typedef struct {
    TAILQ_HEAD(, channel_entry) channels; // doubly-linked list of channels
    UA_UInt32 currentChannelCount;
    UA_UInt32 lastChannelId;
    UA_UInt32 lastTokenId;
    UA_Server* server;
} UA_SecureChannelManager;

typedef enum {
    UA_SECURECHANNELSTATE_FRESH,
    UA_SECURECHANNELSTATE_OPEN,
    UA_SECURECHANNELSTATE_CLOSED
} UA_SecureChannelState;

typedef TAILQ_HEAD(UA_MessageQueue, UA_Message) UA_MessageQueue;

struct UA_SecureChannel {
    UA_SecureChannelState   state;
    UA_MessageSecurityMode  securityMode;
    /* We use three tokens because when switching tokens the client is allowed to accept
     * messages with the old token for up to 25% of the lifetime after the token would have timed out.
     * For messages that are sent, the new token is already used, which is contained in the securityToken
     * variable. The nextSecurityToken variable holds a newly issued token, that will be automatically
     * revolved into the securityToken variable. This could be done with two variables, but would require
     * greater changes to the current code. This could be done in the future after the client and networking
     * structure has been reworked, which would make this easier to implement. */
    UA_ChannelSecurityToken securityToken; /* the channelId is contained in the securityToken */
    UA_ChannelSecurityToken nextSecurityToken;
    UA_ChannelSecurityToken previousSecurityToken;

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

    LIST_HEAD(, UA_SessionHeader) sessions;
    UA_MessageQueue messages;
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

                          /* Nodestore */
    void* nsCtx;

    UA_ServerLifecycle state;

    /* Security */
    UA_SecureChannelManager secureChannelManager;
    UA_SessionManager sessionManager;
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
#endif

    /* Publish/Subscribe */
#ifdef UA_ENABLE_PUBSUB
    UA_PubSubManager pubSubManager;
#endif
};

extern "C" UA_StatusCode
UA_SecureChannelManager_init(UA_SecureChannelManager* cm, UA_Server* server);

/* Remove a all securechannels */
extern "C" void
UA_SecureChannelManager_deleteMembers(UA_SecureChannelManager* cm);

extern "C" UA_StatusCode
UA_SessionManager_init(UA_SessionManager* sm, UA_Server* server);

/* Deletes all sessions */
extern "C" void UA_SessionManager_deleteMembers(UA_SessionManager* sm);


/*********************************************************************************************
Copied from open62541, to be able to implement:

QUaServer::anonymousLoginAllowed
QUaServer::setAnonymousLoginAllowed
set AccessControlContext::allowAnonymous
*/

typedef struct {
    UA_Boolean                allowAnonymous;
    size_t                    usernamePasswordLoginSize;
    UA_UsernamePasswordLogin* usernamePasswordLogin;
} AccessControlContext;
#define ANONYMOUS_POLICY "open62541-anonymous-policy"
#define USERNAME_POLICY  "open62541-username-policy"
const UA_String anonymous_policy = UA_STRING_STATIC(ANONYMOUS_POLICY);
const UA_String username_policy = UA_STRING_STATIC(USERNAME_POLICY);