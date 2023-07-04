#include "quaserver_anex.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
#include <QUaBaseEvent>
#include <QUaGeneralModelChangeEvent>
#include <QUaSystemEvent>
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#include <QUaConditionVariable>
#include <QUaStateVariable>
#include <QUaTwoStateVariable>
#include <QUaFiniteStateVariable>
#include <QUaTransitionVariable>
#include <QUaFiniteTransitionVariable>
#include <QUaCondition>
#include <QUaAcknowledgeableCondition>
#include <QUaAlarmCondition>
#include <QUaStateMachine>
#include <QUaFiniteStateMachine>
#include <QUaState>
#include <QUaTransition>
#include <QUaExclusiveLimitStateMachine>
#include <QUaDiscreteAlarm>
#include <QUaOffNormalAlarm>
#include <QUaLimitAlarm>
#include <QUaExclusiveLimitAlarm>
#include <QUaExclusiveLevelAlarm>
#include <QUaRefreshStartEvent>
#include <QUaRefreshEndEvent>
#include <QUaRefreshRequiredEvent>
#include <QUaTransitionEvent>
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
#include <QUaOptionSetVariable>
#endif

#include <QMetaProperty>
#include <QTimer>

/* helper null log for avoiding startup messages */
void UA_Log_Discard_log(void *context,
                        UA_LogLevel level,
                        UA_LogCategory category,
                        const char *msg,
                        va_list args)
{
    // do nothing
    Q_UNUSED(context);
    Q_UNUSED(level);
    Q_UNUSED(category);
    Q_UNUSED(msg);
    Q_UNUSED(args);
};


UA_StatusCode QUaServer::uaConstructor(UA_Server       * server, 
	                                   const UA_NodeId * sessionId, 
	                                   void            * sessionContext, 
	                                   const UA_NodeId * typeNodeId, 
	                                   void            * typeNodeContext, 
	                                   const UA_NodeId * nodeId, 
	                                   void            ** nodeContext)
{
	Q_UNUSED(server);
	Q_UNUSED(sessionContext); 
	// get server from context object
#ifdef QT_DEBUG 
	auto srv = qobject_cast<QUaServer*>(static_cast<QObject*>(typeNodeContext));
	Q_CHECK_PTR(srv);
#else
	auto srv = static_cast<QUaServer*>(typeNodeContext);
#endif // QT_DEBUG 
	if (!srv)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	// check session (objects can be created or destroyed without client connected)
	//Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	srv->m_currentSession = srv->m_hashSessions.contains(*sessionId) ?
		srv->m_hashSessions[*sessionId] : nullptr;
	// get method from type constructors map and call it
	Q_ASSERT(srv->m_hashConstructors.contains(*typeNodeId));
	auto st = srv->m_hashConstructors[*typeNodeId](nodeId, nodeContext);
	// emit new instance signal if appropriate
	if (srv->m_hashSignalers.contains(*typeNodeId))
	{
		auto signaler = srv->m_hashSignalers.value(*typeNodeId);
#ifdef QT_DEBUG 
		auto newInstance = qobject_cast<QUaNode*>(static_cast<QObject*>(*nodeContext));
		Q_CHECK_PTR(newInstance);
#else
		auto newInstance = static_cast<QUaNode*>(*nodeContext);
#endif // QT_DEBUG 
		emit signaler->signalNewInstance(newInstance);
	}
	return st;
}

void QUaServer::uaDestructor(UA_Server       * server, 
	                         const UA_NodeId * sessionId, 
	                         void            * sessionContext, 
	                         const UA_NodeId * typeNodeId, 
	                         void            * typeNodeContext, 
	                         const UA_NodeId * nodeId, 
	                         void            ** nodeContext)
{
	Q_UNUSED(nodeContext);
	Q_UNUSED(typeNodeContext);
	Q_UNUSED(typeNodeId);
	Q_UNUSED(sessionContext);
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
	// check session (objects can be created or destroyed without client connected)
	//Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	srv->m_currentSession = srv->m_hashSessions.contains(*sessionId) ?
		srv->m_hashSessions[*sessionId] : nullptr;
	// get node
	void * context;
	st = UA_Server_getNodeContext(server, *nodeId, &context);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// try to convert to node (NOTE : nullptr is triggered by ~QUaNode)
#ifdef QT_DEBUG 
	auto node = qobject_cast<QUaNode*>(static_cast<QObject*>(context));
#else
	auto node = static_cast<QUaNode*>(context);
#endif // QT_DEBUG 
	// early exit if not convertible (this call was triggered by ~QUaNode)
	if (!node)
	{
		return;
	}
	// handle events if enabled
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// check if event (not in tree)
	auto evt = qobject_cast<QUaBaseEvent*>(node);
	if (evt)
	{
		evt->deleteLater();
		return;
	}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// if convertible could mean:
	// 1) this is a child of a node being deleted programatically 
	//    this one does not require to call C++ delete because Qt will take care of it
	// 2) this is a node being deleted from the network
	//    this one requires C++ delete
	// we can differentiate them because parent of 1) would not have a context
	// in which case we set current child context to nullptr to continue pattern and return
	UA_NodeId parentNodeId = QUaNode::getParentNodeId(*nodeId, server);
	Q_ASSERT(!UA_NodeId_equal(&parentNodeId, &UA_NODEID_NULL));
	void * parentContext;
	st = UA_Server_getNodeContext(server, parentNodeId, &parentContext);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	auto parentNode = qobject_cast<QUaNode*>(static_cast<QObject*>(parentContext));
	if (!parentNode)
	{
		st = UA_Server_setNodeContext(server, *nodeId, nullptr);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		UA_NodeId_clear(&parentNodeId);
		return;
	}
	// if we reach here it means case 2), so deleteLater (when nodeId not in store anymore)
	// so we avoid calling UA_Server_deleteNode in ~QUaNode
	node->deleteLater();
	Q_UNUSED(st);
	UA_NodeId_clear(&parentNodeId);
}

// try to make instance declaration nodeIds a little more predictable
UA_StatusCode QUaServer::generateChildNodeId(
	UA_Server       *server, 
	const UA_NodeId *sessionId, 
	void            *sessionContext, 
	const UA_NodeId *sourceNodeId, 
	const UA_NodeId *targetParentNodeId, 
	const UA_NodeId *referenceTypeId, 
	UA_NodeId       *targetNodeId)
{
	Q_UNUSED(sessionId);
	Q_UNUSED(sessionContext);
	Q_UNUSED(referenceTypeId);
	// read browse name
	UA_QualifiedName outBrowseName;
	auto st = UA_Server_readBrowseName(server, *sourceNodeId, &outBrowseName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	auto qserver = QUaServer::getServerNodeContext(server);
	// if parent has string node id try to add child also as string
	if (qserver->m_childNodeIdCallback)
	{
		// get child browse name
		QUaQualifiedName childBrowseName = QUaQualifiedName(outBrowseName);		
		// get parent node id
		QUaNodeId parentNodeId = QUaNodeId(*targetParentNodeId);
		// call callback
		QUaNodeId childNodeId = qserver->m_childNodeIdCallback(parentNodeId, childBrowseName);	
		bool isNull = childNodeId.isNull();
		bool isUsed = false;
		if (!isNull)
		{
			isUsed = qserver->isNodeIdUsed(childNodeId);
		}		
		// copy if success
		if (!isNull && !isUsed)
		{
			// copy
			*targetNodeId = childNodeId;
			// cleanup
			UA_QualifiedName_clear(&outBrowseName);
			return UA_STATUSCODE_GOOD;
		}
		// log errors
		if (isNull)
		{
			emit qserver->logMessage({
				tr("Child %1 nodeId for parent %2 is null. Assigning random nodeId.")
				.arg(childBrowseName.name()).arg(parentNodeId),
				QUaLogLevel::Error,
				QUaLogCategory::UserLand
			});
		}
		// log errors
		if (isUsed)
		{
			emit qserver->logMessage({
				tr("Child %1 nodeId for parent %2 is already in use. Assigning random nodeId.")
				.arg(childBrowseName.name()).arg(parentNodeId),
				QUaLogLevel::Error,
				QUaLogCategory::UserLand
			});
		}
	}
	// get parent node id hash
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	uint parentHash = qHash(*targetParentNodeId, targetParentNodeId->namespaceIndex);
#else
	size_t parentHash = qHash(*targetParentNodeId, targetParentNodeId->namespaceIndex);
#endif
	// get child browse name hash
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	uint childNameHash = qHash(
#else
	size_t childNameHash = qHash(
#endif
		QByteArray::fromRawData((const char*)outBrowseName.name.data, static_cast<int>(outBrowseName.name.length)),
			outBrowseName.namespaceIndex
		);
	// new node id is combination of both
	targetNodeId->identifierType = UA_NODEIDTYPE_NUMERIC;
	targetNodeId->identifier.numeric = parentHash ^ childNameHash;
	// cleanup
	UA_QualifiedName_clear(&outBrowseName);
	// check 	
	while (qserver->isNodeIdUsed(*targetNodeId))
	{
		targetNodeId->identifier.numeric = targetNodeId->identifier.numeric ^ childNameHash;
	}
	return UA_STATUSCODE_GOOD;
}

UA_StatusCode QUaServer::uaConstructor(
	QUaServer         *server,
	const UA_NodeId   *nodeId, 
	void             **nodeContext,
	const QMetaObject &metaObject
)
{
	// get parent node id
	UA_NodeId topBoundParentNodeId = QUaNode::getParentNodeId(*nodeId, server->m_server);
	// find top level node which is bound (bound := NodeId context == QUaNode instance)
	UA_NodeClass outNodeClass;
	QUaNode * parentContext = nullptr;
	// NOTE : not only events can be parent-less (e.g. conditions)
	while (!UA_NodeId_isNull(&topBoundParentNodeId))
	{
		// handle events
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		// check if event
		if (metaObject.inherits(&QUaBaseEvent::staticMetaObject)
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
			&& !metaObject.inherits(&QUaCondition::staticMetaObject)
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
			)
		{
			break;
		}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
		// ignore if new instance is due to new type registration
		UA_Server_readNodeClass(server->m_server, topBoundParentNodeId, &outNodeClass);
		Q_ASSERT_X(outNodeClass != UA_NODECLASS_UNSPECIFIED, "QUaServer::uaConstructor", 
			"Something went wrong while getting parent info.");
		if(outNodeClass != UA_NODECLASS_OBJECT && outNodeClass != UA_NODECLASS_VARIABLE)
		{
			UA_NodeId_clear(&topBoundParentNodeId);
			return UA_STATUSCODE_GOOD;
		}
		parentContext = QUaNode::getNodeContext(topBoundParentNodeId, server->m_server);
		// check if parent node is bound
		if (parentContext)
		{
			break;
		}
		// try next parent in hierarchy
		auto tmpParentNodeId = QUaNode::getParentNodeId(topBoundParentNodeId, server->m_server);
		UA_NodeId_clear(&topBoundParentNodeId); // clear old
		topBoundParentNodeId = tmpParentNodeId; // shallow copy
	}
	// create new instance (and bind it to UA, in base types happens in constructor, 
	// in derived class is done by QOpcUaServerNodeFactory)
	Q_ASSERT_X(metaObject.constructorCount() > 0, "QUaServer::uaConstructor", 
		"Failed instantiation. No matching Q_INVOKABLE constructor with signature "
		"CONSTRUCTOR(QUaServer *server) found.");
	// NOTE : to simplify user API, we minimize QUaNode arguments to just a QUaServer 
	// reference we temporarily store in the QUaServer reference the UA_NodeId and 
	// QMetaObject values needed to instantiate the new node.
	server->m_newNodeNodeId     = nodeId;
	server->m_newNodeMetaObject = &metaObject;
	// instantiate new C++ node, m_newNodeNodeId and m_newNodeMetaObject only meant to be used during this call
	auto * pQObject = metaObject.newInstance(Q_ARG(QUaServer*, server));
	Q_ASSERT_X(pQObject, "QUaServer::uaConstructor", 
		"Failed instantiation. No matching Q_INVOKABLE constructor with signature "
		"CONSTRUCTOR(QUaServer *server) found.");
	auto* newInstance = qobject_cast<QUaNode*>(pQObject);
	Q_CHECK_PTR(newInstance);
	if (!newInstance)
	{
		UA_NodeId_clear(&topBoundParentNodeId);
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	// need to bind again using the official (void ** nodeContext) of the UA constructor
	// because we set context on C++ instantiation, but later the UA library overwrites it 
	// after calling the UA constructor
	*nodeContext = static_cast<void*>(newInstance);
	newInstance->m_nodeId = *nodeId;
	// need to set parent if direct parent is already bound bacause its constructor has already been called
	UA_NodeId directParentNodeId = QUaNode::getParentNodeId(*nodeId, server->m_server);
	if (parentContext && UA_NodeId_equal(&topBoundParentNodeId, &directParentNodeId))
	{
		auto browseName = QUaNode::getBrowseName(*nodeId, server->m_server);
		newInstance->setParent(parentContext);
		newInstance->setObjectName(browseName);
		Q_ASSERT(!parentContext->browseChild(browseName));
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
		uint key = qHash(browseName);
#else
		size_t key = qHash(browseName);
#endif
		parentContext->m_browseCache[key] = newInstance;
		QObject::connect(newInstance, &QObject::destroyed, parentContext, [parentContext, key]() {
			parentContext->m_browseCache.remove(key);
		});	
		// emit child added to parent
		emit parentContext->childAdded(newInstance);
	}
	// success
	UA_NodeId_clear(&topBoundParentNodeId);
	UA_NodeId_clear(&directParentNodeId);
	return UA_STATUSCODE_GOOD;
}

// [STATIC]
UA_StatusCode QUaServer::methodCallback(
	UA_Server        *server,
	const UA_NodeId  *sessionId, 
	void             *sessionContext, 
	const UA_NodeId  *methodId, 
	void             *methodContext, 
	const UA_NodeId  *objectId, 
	void             *objectContext, 
	size_t            inputSize, 
	const UA_Variant *input, 
	size_t            outputSize, 
	UA_Variant       *output
)
{
	Q_UNUSED(server        );
	Q_UNUSED(sessionContext);
	Q_UNUSED(objectId      );
	Q_UNUSED(inputSize     );
	Q_UNUSED(outputSize    );
	// get node from context object
#ifdef QT_DEBUG 
	auto srv = qobject_cast<QUaServer*>(static_cast<QObject*>(methodContext));
	Q_CHECK_PTR(srv);
#else
	auto srv = static_cast<QUaServer*>(methodContext);
#endif // QT_DEBUG 
	if (!srv)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	// check session
	Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	srv->m_currentSession = srv->m_hashSessions.contains(*sessionId) ?
		srv->m_hashSessions[*sessionId] : nullptr;
	// check method
	Q_ASSERT(srv->m_hashMethods.contains(*methodId));
	if (!srv->m_hashMethods.contains(*methodId))
	{
		return (UA_StatusCode)UA_STATUSCODE_BADINTERNALERROR;
	}
	// get method from node callbacks map and call it
	return srv->m_hashMethods[*methodId](objectContext, input, output);
}

UA_StatusCode QUaServer::callMetaMethod(
	QUaServer         *server, 
	QUaBaseObject     *object, 
	const QMetaMethod &metaMethod, 
	const UA_Variant  *input, 
	UA_Variant        *output)
{
	// convert input arguments to QVariants
	QVariantList varListArgs;
	QList<QGenericArgument> genListArgs;
	auto listTypeNames = metaMethod.parameterTypes();
	for (int k = 0; k < metaMethod.parameterCount(); k++)
	{
		QVariant varArg;
		auto metaType    = (QMetaType::Type)metaMethod.parameterType(k);
		auto strTypeName = QString(listTypeNames.at(k));
		// NOTE : enums are QMetaType::UnknownType
		Q_ASSERT_X(metaType != QMetaType::UnknownType ||
			server->m_hashEnums.contains(strTypeName),
			"QUaServer::callMetaMethod",
			"Argument type is not registered. Try using qRegisterMetaType.");
		if (metaType == QMetaType::UnknownType &&
			!server->m_hashEnums.contains(strTypeName))
		{
			return (UA_StatusCode)UA_STATUSCODE_BADINTERNALERROR;
		}
		if (strTypeName.contains(QLatin1String("QList"), Qt::CaseInsensitive))
		{
			varArg = QUaTypesConverter::uaVariantToQVariantArray(input[k],
				QUaTypesConverter::ArrayType::QList);
		}
		else if (strTypeName.contains(QLatin1String("QVector"), Qt::CaseInsensitive))
		{
			varArg = QUaTypesConverter::uaVariantToQVariantArray(input[k],
				QUaTypesConverter::ArrayType::QVector);
		}
		else
		{
			varArg = QUaTypesConverter::uaVariantToQVariant(input[k]);
		}
		// NOTE : need to keep in stack while method is being called
		varListArgs.append(varArg);
		// create generic argument only with reference to data
		genListArgs.append(QGenericArgument(
			QMetaType( varListArgs[k].userType() ).name(),
			const_cast<void*>(varListArgs[k].constData())
		));
	}
	// create return QVariant
	int retType = metaMethod.returnType();
	// NOTE : enums are QMetaType::UnknownType
	Q_ASSERT_X(retType != QMetaType::UnknownType ||
		server->m_hashEnums.contains(metaMethod.typeName()),
		"QUaServer::callMetaMethod",
		"Return type is not registered. Try using qRegisterMetaType."
	);
	if (retType == QMetaType::UnknownType)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADINTERNALERROR;
	}
	QVariant returnValue;
	// avoid Qt printing warning to console
	if (retType != QMetaType::Void)
	{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
		returnValue = QVariant(retType);
#else
		returnValue = QVariant( QMetaType(retType) );
#endif
	}
	QGenericReturnArgument returnArgument(
		metaMethod.typeName(),
		const_cast<void*>(returnValue.constData())
	);
	// reset status code
	server->m_methodRetStatusCode = UA_STATUSCODE_GOOD;
	// call metaMethod
	bool ok = metaMethod.invoke(
		object,
		Qt::DirectConnection,
		returnArgument,
		genListArgs.value(0),
		genListArgs.value(1),
		genListArgs.value(2),
		genListArgs.value(3),
		genListArgs.value(4),
		genListArgs.value(5),
		genListArgs.value(6),
		genListArgs.value(7),
		genListArgs.value(8),
		genListArgs.value(9)
	);
	Q_ASSERT(ok);
	Q_UNUSED(ok);
	// set return value if any
	if (retType != QMetaType::Void)
	{
		UA_Variant tmpVar = QUaTypesConverter::uaVariantFromQVariant(returnValue);
		// TODO : have to cleanup? UA_Variant_deleteMembers(&tmpVar)
		*output = tmpVar;
	}
	// copy return status code
	UA_StatusCode retStatusCode = server->m_methodRetStatusCode;
	// reset status code
	server->m_methodRetStatusCode = UA_STATUSCODE_GOOD;
	// return success status
	return retStatusCode;
}

void QUaServer::bindMethod(
	QUaServer       *server, 
	const UA_NodeId *methodNodeId, 
	const int       &metaMethodIndex)
{
	// register callback in open62541
	auto st = UA_Server_setMethodNode_callback(server->m_server, *methodNodeId, &QUaServer::methodCallback);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// set QUaServer* instance as method context
	st = UA_Server_setNodeContext(
		server->m_server,
		*methodNodeId,
		static_cast<void*>(server)
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	Q_ASSERT_X(!server->m_hashMethods.contains(*methodNodeId), "QUaServer::registerTypeDefaults",
		"Cannot register same method more than once.");
	if (server->m_hashMethods.contains(*methodNodeId))
	{
		return;
	}
	// NOTE : had to cache the method's index in lambda capture because caching 
	//        metaMethod directly was not working in some cases the internal data
	//        of the metaMethod was deleted which resulted in access violation
	server->m_hashMethods[*methodNodeId] = [metaMethodIndex, server](
		void* objectContext,
		const UA_Variant* input,
		UA_Variant* output)
	{
		// get object instance that owns method
#ifdef QT_DEBUG 
		QUaBaseObject* object = qobject_cast<QUaBaseObject*>(static_cast<QObject*>(objectContext));
		Q_ASSERT_X(object,
			"QUaServer::bindMethod",
			"Cannot call method on invalid C++ object.");
#else
		QUaBaseObject* object = static_cast<QUaBaseObject*>(objectContext);
#endif // QT_DEBUG 
		if (!object)
		{
			return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
		}
		// get meta method
		auto metaMethod = object->metaObject()->method(metaMethodIndex);
		return QUaServer::callMetaMethod(server, object, metaMethod, input, output);
	};
}

QHash<QUaQualifiedName, int> QUaServer::metaMethodIndexes(const QMetaObject& metaObject)
{
	QHash<QUaQualifiedName, int> retHash;
	for (int methIdx = metaObject.methodOffset(); methIdx < metaObject.methodCount(); methIdx++)
	{
		QMetaMethod metaMethod = metaObject.method(methIdx);
		// validate id method (not signal, slot or constructor)
		auto methodType = metaMethod.methodType();
		if (methodType != QMetaMethod::Method)
		{
			continue;
		}
		// add method
		QUaQualifiedName methName = QString(metaMethod.name());
		retHash[methName] = methIdx;
	}
	return retHash;
}

bool QUaServer::isNodeBound(const UA_NodeId & nodeId, UA_Server *server)
{
	auto ptr = QUaNode::getNodeContext(nodeId, server);
	if (!ptr)
	{
		return false;
	}
	// test if nodeId assigned to context
	if (!UA_NodeId_equal(&nodeId, &ptr->m_nodeId))
	{
		return false;
	}
	// success
	return true;
}

extern "C" {
	typedef UA_StatusCode(*UA_exchangeEncodeBuffer)(void *handle, UA_Byte **bufPos,
		const UA_Byte **bufEnd);

	UA_EXPORT extern UA_StatusCode
		UA_encodeBinary(const void *src, const UA_DataType *type,
			UA_Byte **bufPos, const UA_Byte **bufEnd,
			UA_exchangeEncodeBuffer exchangeCallback,
			void *exchangeHandle) UA_FUNC_ATTR_WARN_UNUSED_RESULT;
}

QUaServer * QUaServer::getServerNodeContext(UA_Server * server)
{
	auto context = QUaNode::getVoidContext(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), server);
	// try to cast to C++ server
#ifdef QT_DEBUG 
	auto srv = qobject_cast<QUaServer*>(static_cast<QObject*>(context));
	Q_CHECK_PTR(srv);
#else
	auto srv = static_cast<QUaServer*>(context);
#endif // QT_DEBUG 
	return srv;
}

QString QUaServer::m_anonUser      = QObject::tr("[Anonymous]");
QString QUaServer::m_anonUserToken = QObject::tr("[Anonymous:EmptyToken]");
QStringList QUaServer::m_anonUsers = QStringList() 
	<< QUaServer::m_anonUser
	<< QUaServer::m_anonUserToken;

// Copied from activateSession_default in open62541 and then modified to use custom stuff
UA_StatusCode QUaServer::activateSession(UA_Server                    * server, 
	                                     UA_AccessControl             * ac, 
	                                     const UA_EndpointDescription * endpointDescription, 
	                                     const UA_ByteString          * secureChannelRemoteCertificate, 
	                                     const UA_NodeId              * sessionId, 
	                                     const UA_ExtensionObject     * userIdentityToken, 
	                                     void                        ** sessionContext)
{
	Q_UNUSED(secureChannelRemoteCertificate);
	Q_UNUSED(endpointDescription);
	AccessControlContext *context = (AccessControlContext*)ac->context;

	// NOTE : custom code : get server instance
	QUaServer *srv = QUaServer::getServerNodeContext(server);
	/* The empty token is interpreted as anonymous */
	if (userIdentityToken->encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY) {
		if (!context->allowAnonymous)
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		// NOTE : custom code : add session to hash
		Q_ASSERT(!srv->m_hashSessions.contains(*sessionId));
        srv->m_hashSessions.insert(*sessionId, new QUaSession(srv));
        srv->m_hashSessions[*sessionId]->m_strUserName = QUaServer::m_anonUserToken;

		// notify client connected
		QUaServer::newSession(srv, sessionId);

		/* No userdata atm */
		*sessionContext = NULL;
		return (UA_StatusCode)UA_STATUSCODE_GOOD;
	}

	/* Could the token be decoded? */
	if (userIdentityToken->encoding < UA_EXTENSIONOBJECT_DECODED)
		return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

	/* Anonymous login */
	if (userIdentityToken->content.decoded.type == &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]) {
		if (!context->allowAnonymous)
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		const UA_AnonymousIdentityToken *token = (UA_AnonymousIdentityToken*)
			userIdentityToken->content.decoded.data;

		/* Compatibility notice: Siemens OPC Scout v10 provides an empty
		 * policyId. This is not compliant. For compatibility, assume that empty
		 * policyId == ANONYMOUS_POLICY */
		if (token->policyId.data && !UA_String_equal(&token->policyId, &anonymous_policy))
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		// NOTE : custom code : add session to hash
		// NOTE : actually is possible for a current session to change its user while maintaining nodeId
		//Q_ASSERT(!srv->m_hashSessions.contains(*sessionId));
        if(!srv->m_hashSessions.contains(*sessionId))
        {
            srv->m_hashSessions.insert(*sessionId, new QUaSession(srv));
        }
        srv->m_hashSessions[*sessionId]->m_strUserName = QUaServer::m_anonUser;

		// notify client connected
		QUaServer::newSession(srv, sessionId);

		/* No userdata atm */
		*sessionContext = NULL;
		return (UA_StatusCode)UA_STATUSCODE_GOOD;
	}

	/* Username and password */
	if (userIdentityToken->content.decoded.type == &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]) {
		const UA_UserNameIdentityToken *userToken =
			(UA_UserNameIdentityToken*)userIdentityToken->content.decoded.data;

		if (!UA_String_equal(&userToken->policyId, &username_policy))
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		/* TODO: Support encrypted username/password over unencrypted SecureChannels 
		// NOTE: Why was this here? Plaintext username and pass should be supported when comms are encrypted. 
		if (userToken->encryptionAlgorithm.length > 0)
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
		*/

		/* Empty username and password */
		if (userToken->userName.length == 0 && userToken->password.length == 0)
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		
		/* Try to match username/pw */

		// NOTE : custom code : check user and password
		const QString userName = QString::fromUtf8((char*)userToken->userName.data, (int)userToken->userName.length);
		const QString password = QString::fromUtf8((char*)userToken->password.data, (int)userToken->password.length);	
		// Call validation callback
		UA_Boolean match = srv->m_validationCallback(userName, password);
		if (!match)
		{
			return UA_STATUSCODE_BADUSERACCESSDENIED;			
		}
		else if (match && !srv->m_hashUsers.contains(userName))
		{
			// Server must be aware of user name otherwise it will kick user out of session
			// in user access callbacks
			// NOTE : do not keep password in memory for security
			srv->m_hashUsers.insert(userName, QString());
		}

		// NOTE : actually is possible for a current session to change its user while maintaining nodeId
		//Q_ASSERT(!srv->m_hashSessions.contains(*sessionId));
        if(!srv->m_hashSessions.contains(*sessionId))
        {
            srv->m_hashSessions.insert(*sessionId, new QUaSession(srv));
        }
		// NOTE : custom code : add session to hash
        srv->m_hashSessions[*sessionId]->m_strUserName = userName;

		// notify client connected
		QUaServer::newSession(srv, sessionId);

		/* No userdata atm */
		*sessionContext = NULL;

		return (UA_StatusCode)UA_STATUSCODE_GOOD;
	}

	/* Unsupported token type */
	return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
}

void QUaServer::newSession(QUaServer* server,
	                       const UA_NodeId* sessionId)
{
	UA_Session* session = UA_Server_getSessionById(server->m_server, sessionId);
	Q_ASSERT(session);
	UA_ApplicationDescription clientDescription = session->clientDescription;
	QString strApplicationUri  = QUaTypesConverter::uaStringToQString(clientDescription.applicationUri);
	QString strProductUri      = QUaTypesConverter::uaStringToQString(clientDescription.productUri);
	QString strApplicationName = QUaTypesConverter::uaVariantToQVariantScalar<QUaLocalizedText, UA_LocalizedText>(&clientDescription.applicationName);
	// get connection data
	QString strAddress;
	quint16 intPort;
	// get connection reference from channel
	// NOTE : when pausing the application on a breakpoint,
	// below condition is true and can crash the session
	if (!session->header.channel)
	{
		return;
	}
	UA_Connection* connection = session->header.channel->connection;
	// get peer name (address) from socket fd
	auto sockFd = connection->sockfd;
	sockaddr address;
	socklen_t address_len = sizeof(address);
	auto res = getpeername(sockFd, &address, &address_len);
	char remote_name[100];
	res = UA_getnameinfo(&address,
		sizeof(struct sockaddr_storage),
		remote_name, sizeof(remote_name),
		NULL, 0, NI_NUMERICHOST);
    Q_UNUSED(res);
	strAddress = QString(remote_name);
	// get peer port
	switch (address.sa_family) 
	{
	case AF_INET: 
		{
			sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(&address);
			intPort = static_cast<quint16>(sin->sin_port);
			break;
		}
	case AF_INET6: 
		{
			sockaddr_in6* sin = reinterpret_cast<sockaddr_in6*>(&address);
			intPort = static_cast<quint16>(sin->sin6_port);
			break;
		}
	default:
		{
			// TODO : [BUG] sometimes when client keeps old session, sockFd is not valid?
			//Q_ASSERT(false);
			strAddress = QStringLiteral("Unknown");
			intPort = 0;
		}

	}
	// store session data
	Q_ASSERT(server->m_hashSessions.contains(*sessionId));
    server->m_hashSessions[*sessionId]->m_strSessionId       = QUaTypesConverter::nodeIdToQString(*sessionId);
    server->m_hashSessions[*sessionId]->m_strApplicationName = strApplicationName;
    server->m_hashSessions[*sessionId]->m_strApplicationUri	 = strApplicationUri;
    server->m_hashSessions[*sessionId]->m_strProductUri		 = strProductUri;
    server->m_hashSessions[*sessionId]->m_strAddress		 = strAddress;
    server->m_hashSessions[*sessionId]->m_intPort			 = intPort;
	// emit new client connected event
	emit server->clientConnected(server->m_hashSessions[*sessionId]);
}

void QUaServer::closeSession(UA_Server        * server, 
	                         UA_AccessControl * ac, 
	                         const UA_NodeId  * sessionId, 
	                         void             * sessionContext)
{
	Q_UNUSED(sessionContext);
	Q_UNUSED(ac);
	// get server
	QUaServer *srv = QUaServer::getServerNodeContext(server);
	// remove session from hash
	if (!srv->m_hashSessions.contains(*sessionId))
	{
		// TODO : failed connection, bad identity log message
		return;
	}
	auto session = srv->m_hashSessions.take(*sessionId);
	emit srv->clientDisconnected(session);
	session->deleteLater();
}

UA_UInt32 QUaServer::getUserRightsMask(UA_Server        *server,
	                                   UA_AccessControl *ac,
	                                   const UA_NodeId  *sessionId,
	                                   void             *sessionContext,
	                                   const UA_NodeId  *nodeId,
	                                   void             *nodeContext) 
{
	Q_UNUSED(nodeContext);
	Q_UNUSED(sessionContext);
	Q_UNUSED(ac);
	// get server
	QUaServer *srv = QUaServer::getServerNodeContext(server);
	Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	// get user
    QString strUserName = srv->m_hashSessions[*sessionId]->m_strUserName;
	// check if user still exists
	if (!srv->userExists(strUserName))
	{
		// TODO : wait until officially supported
		// https://github.com/open62541/open62541/issues/2617
		//auto st = UA_Server_closeSession(server, sessionId);
		//Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return (UA_UInt32)0;
	}
	// if node from user tree then call user implementation
	QUaNode * node = QUaNode::getNodeContext(*nodeId, server);
	if (node)
	{
		return node->userWriteMaskInternal(strUserName).intValue;
	}
	// else default
	return 0xFFFFFFFF;
}

UA_Byte QUaServer::getUserAccessLevel(UA_Server        *server,
	                                  UA_AccessControl *ac,
	                                  const UA_NodeId  *sessionId,
	                                  void             *sessionContext,
	                                  const UA_NodeId  *nodeId,
	                                  void             *nodeContext)
{
	Q_UNUSED(nodeContext);
	Q_UNUSED(sessionContext);
	Q_UNUSED(ac);
	// get server
	QUaServer *srv = QUaServer::getServerNodeContext(server);
	Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	// get user
    QString strUserName = srv->m_hashSessions[*sessionId]->m_strUserName;
	// check if user still exists
	if (!srv->userExists(strUserName))
	{
		// TODO : wait until officially supported
		// https://github.com/open62541/open62541/issues/2617
		//auto st = UA_Server_closeSession(server, sessionId);
		//Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return (UA_UInt32)0;
	}
	// if node from user tree then call user implementation
	QUaNode * node = QUaNode::getNodeContext(*nodeId, server);
	QUaBaseVariable * variable = qobject_cast<QUaBaseVariable *>(node);
	if (variable)
	{
		return variable->userAccessLevelInternal(strUserName).intValue;
	}
	// else default
	return 0xFF;
}

// NOTE : called when reading attributes
UA_Boolean QUaServer::getUserExecutable(UA_Server        *server, 
		                                UA_AccessControl *ac,
		                                const UA_NodeId  *sessionId, 
		                                void             *sessionContext,
		                                const UA_NodeId  *methodId, 
		                                void             *methodContext)
{
	Q_UNUSED(methodContext);
	Q_UNUSED(methodId);
	Q_UNUSED(sessionContext);
	Q_UNUSED(ac);
	// overall execution permissions for method regardless of conntext object
	// boils down to whether user exists
	// get server
	QUaServer *srv = QUaServer::getServerNodeContext(server);
	Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	// get user
    QString strUserName = srv->m_hashSessions[*sessionId]->m_strUserName;
	// check if user still exists
	if (!srv->userExists(strUserName))
	{
		// TODO : wait until officially supported
		// https://github.com/open62541/open62541/issues/2617
		//auto st = UA_Server_closeSession(server, sessionId);
		//Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return false;
	}
	return true;
}

// NOTE : called when actually requesting to execute method
UA_Boolean QUaServer::getUserExecutableOnObject(UA_Server        *server, 
		                                        UA_AccessControl *ac,
		                                        const UA_NodeId  *sessionId, 
		                                        void             *sessionContext,
		                                        const UA_NodeId  *methodId, 
		                                        void             *methodContext,
		                                        const UA_NodeId  *objectId, 
		                                        void             *objectContext)
{
	Q_UNUSED(objectContext);
	Q_UNUSED(methodContext);
	Q_UNUSED(methodId);
	Q_UNUSED(sessionContext);
	Q_UNUSED(ac);
	// get server
	QUaServer *srv = QUaServer::getServerNodeContext(server);
	Q_ASSERT(srv->m_hashSessions.contains(*sessionId));
	// get user
    QString strUserName = srv->m_hashSessions[*sessionId]->m_strUserName;
	// check if user still exists
	if (!srv->userExists(strUserName))
	{
		// TODO : wait until officially supported
		// https://github.com/open62541/open62541/issues/2617
		//auto st = UA_Server_closeSession(server, sessionId);
		//Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return false;
	}
	// if node from user tree then call user implementation
	QUaNode * node = QUaNode::getNodeContext(*objectId, server);
	QUaBaseObject * object = qobject_cast<QUaBaseObject *>(node);
	if (object)
	{
		// NOTE : could not diff by method name because name multiples are possible
		return object->userExecutableInternal(strUserName);
	}
	// else default
	return true;
}

QUaServer::QUaServer(QObject* parent/* = 0*/)
	: QObject(parent)
{
	// defaults
	m_beingDestroyed = false;
	m_port = 4840;
	m_anonymousLoginAllowed = true;
	m_byteCertificate = QByteArray();
	m_byteCertificateInternal = QByteArray();
	m_methodRetStatusCode = UA_STATUSCODE_GOOD;
	m_childNodeIdCallback = nullptr;
#ifdef UA_ENABLE_ENCRYPTION
	m_bytePrivateKey = QByteArray();
	m_bytePrivateKeyInternal = QByteArray();
#endif
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	m_conditionsRefreshRequired = false;
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#ifdef UA_ENABLE_HISTORIZING
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	m_maxHistoryEventResponseSize = 1000;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
#endif // UA_ENABLE_HISTORIZING
	// create long-living open62541 server instance
	this->m_server = UA_Server_new();
	// register custom types to be used with Qt (QVariant and stuff)
	QUaTypesConverter::registerCustomTypes();
	// setup server (other defaults)
	this->setupServer();
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
void QUaServer::addChange(const QUaChangeStructureDataType& change)
{
	// NOTE : do not check if server is running because we might wanna
	//        historize offline events
	if (m_listChanges.contains(change))
	{
		return;
	}
	m_listChanges.append(change);
	// if trigger already scheduled, then ealry exit
	if (m_changeEventSignaler.processing())
	{
		return;
	}
	// exec trigger on next event loop iteration
	m_changeEventSignaler.execLater([this]() {
		// trigger
		auto time = QDateTime::currentDateTimeUtc();
		m_changeEvent->setChanges(m_listChanges);
		m_changeEvent->setTime(time);
		m_changeEvent->setReceiveTime(time);
		m_changeEvent->trigger();
		// clean list of changes buffer
		m_listChanges.clear();
		m_changeEvent->setChanges(m_listChanges);
	});
}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
void QUaServer::requireConditionsRefresh(const QUaLocalizedText& message/* = QUaLocalizedText()*/)
{
	// do net send multiple refresh required events in a single Qt event loop iteration
	if (m_conditionsRefreshRequired)
	{
		return;
	}
	// set flag
	m_conditionsRefreshRequired = true;
	// schedule refresh required event
	auto time = QDateTime::currentDateTimeUtc();
	m_refreshRequiredEvent->setEventId(QUaBaseEvent::generateEventId());
	m_refreshRequiredEvent->setTime(time);
	m_refreshRequiredEvent->setReceiveTime(time);
	m_refreshRequiredEvent->setMessage(message);
	// exec trigger on next event loop iteration
	m_changeEventSignaler.execLater([this]() {
		// trigger
		m_refreshRequiredEvent->trigger();
		// reset flag
		m_conditionsRefreshRequired = false;
	});
}
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#ifdef UA_ENABLE_HISTORIZING
UA_HistoryDataGathering QUaServer::getGathering() const
{
	return static_cast<UA_HistoryDatabaseContext_default*>(m_historDatabase.context)->gathering;
}
quint8 QUaServer::eventNotifier() const
{
	UA_Byte outByte;
	auto st = UA_Server_readEventNotifier(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), &outByte);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outByte;
}
void QUaServer::setEventNotifier(const quint8& eventNotifier)
{
	auto st = UA_Server_writeEventNotifier(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), eventNotifier);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// TODO : emit signal?	
}
#endif // UA_ENABLE_HISTORIZING

void QUaServer::resetConfig()
{
	// clean old config and create new
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	// NOTE : cannot call UA_ServerConfig_clean because it cleans node store
	//        so we just call the parts that interest us
	//UA_ServerConfig_clean(config);
	/* Network Layers */
	for (size_t i = 0; i < config->networkLayersSize; ++i)
		config->networkLayers[i].clear(&config->networkLayers[i]);
	UA_free(config->networkLayers);
	config->networkLayers = NULL;
	config->networkLayersSize = 0;
    UA_String_clear(&config->customHostname);
	config->customHostname = UA_STRING_NULL;
	/* Security Policy */
	for (size_t i = 0; i < config->securityPoliciesSize; ++i) {
		UA_SecurityPolicy* policy = &config->securityPolicies[i];
		policy->clear(policy);
	}
	UA_free(config->securityPolicies);
	config->securityPolicies = NULL;
	config->securityPoliciesSize = 0;
	/* Endoints */
	for (size_t i = 0; i < config->endpointsSize; ++i)
        UA_EndpointDescription_clear(&config->endpoints[i]);
	UA_free(config->endpoints);
	config->endpoints = NULL;
	config->endpointsSize = 0;
	/* Certificate Validation */
	if (config->certificateVerification.clear)
		config->certificateVerification.clear(&config->certificateVerification);
	/* Access Control */
	if (config->accessControl.clear)
		config->accessControl.clear(&config->accessControl);

	UA_StatusCode st;
#ifndef UA_ENABLE_ENCRYPTION
	// convert cert if valid
	UA_ByteString cert;
	UA_ByteString* ptrCert = QUaServer::parseCertificate(m_byteCertificate, cert, m_byteCertificateInternal);
	st = UA_ServerConfig_setMinimal(config, m_port, ptrCert);
#else
	// convert cert if valid (should contain public key)
	UA_ByteString cert;
	UA_ByteString* ptrCert = QUaServer::parseCertificate(m_byteCertificate, cert, m_byteCertificateInternal);
	// convert private key if valid
	UA_ByteString priv;
	UA_ByteString* ptrPriv = QUaServer::parseCertificate(m_bytePrivateKey, priv, m_bytePrivateKeyInternal);
	// check if valid private key
	if (ptrPriv)
	{
		// create config with port, certificate and private key for encryption
		st = UA_ServerConfig_setDefaultWithSecurityPolicies(
			config,
			m_port,
			ptrCert,
			ptrPriv,
			nullptr,
			0,
			nullptr,
			0,
			nullptr,
			0
		);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
	}
	else
	{
		// create config with port and certificate only (no encryption)
		st = UA_ServerConfig_setMinimal(config, m_port, ptrCert);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
	}
#endif
	
	// setup logger
	config->logger = this->getLogger();

	// setup access control
	AccessControlContext* context = static_cast<AccessControlContext*>(config->accessControl.context);
	context->allowAnonymous = m_anonymousLoginAllowed;

	// static methods to reimplement custom behaviour
	config->accessControl.activateSession           = &QUaServer::activateSession;
	config->accessControl.closeSession              = &QUaServer::closeSession;
	config->accessControl.getUserRightsMask         = &QUaServer::getUserRightsMask;
	config->accessControl.getUserAccessLevel        = &QUaServer::getUserAccessLevel;
	config->accessControl.getUserExecutable         = &QUaServer::getUserExecutable;
	config->accessControl.getUserExecutableOnObject = &QUaServer::getUserExecutableOnObject;

	// TODO : implement rest of callbacks
	//        allowAddNode_default
	//        allowAddReference_default
	//        allowDeleteNode_default
	//        allowDeleteReference_default

	// setup server description

	config->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC((char*)"", m_byteApplicationName.data());
	config->applicationDescription.applicationUri  = UA_String_fromChars(m_byteApplicationUri.data());
	config->buildInfo.productName                  = UA_String_fromChars(m_byteProductName.data());
	config->buildInfo.productUri                   = UA_String_fromChars(m_byteProductUri.data());
	config->buildInfo.manufacturerName             = UA_String_fromChars(m_byteManufacturerName.data());
	config->buildInfo.softwareVersion              = UA_String_fromChars(m_byteSoftwareVersion.data());
	config->buildInfo.buildNumber                  = UA_String_fromChars(m_byteBuildNumber.data());
	// NOTE : update application description productUri as well
	config->applicationDescription.productUri      = UA_String_fromChars(m_byteProductUri.data());
	// update endpoints necessary
	// NOTE : use about updating config->applicationDescription.
	//        In UA_ServerConfig_new_customBuffer we have
	//        conf->endpointsSize = 1;
	st = UA_ApplicationDescription_copy(&config->applicationDescription, &config->endpoints[0].server);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// setup server limits

	config->maxSecureChannels = m_maxSecureChannels;
	config->maxSessions       = m_maxSessions;

#ifdef UA_ENABLE_HISTORIZING
	config->historyDatabase = m_historDatabase;
#endif // UA_ENABLE_HISTORIZING

	// custom instance declaration NodeId mechanism
	config->nodeLifecycle.generateChildNodeId = &QUaServer::generateChildNodeId;

	Q_UNUSED(st);
}

UA_ByteString * QUaServer::parseCertificate(const QByteArray &inByteCert,
	                                        UA_ByteString    &outUaCert,
	                                        QByteArray       &outByteCertt)
{
	UA_ByteString *ptr = nullptr;
	if (!inByteCert.isEmpty())
	{
		outByteCertt = inByteCert;
		// convert QByteArray to UA_ByteString
		size_t          cert_length = static_cast<size_t>(outByteCertt.length());
		const UA_Byte * cert_data = reinterpret_cast<const UA_Byte *>(outByteCertt.constData());
		outUaCert.length = cert_length;
		UA_StatusCode success = UA_Array_copy(
			cert_data,                                  // src
			cert_length,                                // size
			reinterpret_cast<void **>(&outUaCert.data), // dst
			&UA_TYPES[UA_TYPES_BYTE]                    // type
		);
		// only set pointer if succeeds
		if (success == UA_STATUSCODE_GOOD)
		{
			ptr = &outUaCert;
		}
	}
	return ptr;
}

void QUaServer::setupServer()
{
	// Server stuff
	UA_StatusCode st;
	m_running = false;
	// Set default validation callback
	m_validationCallback = [this](const QString& strUserName, const QString& strPassword) {
		if (!m_hashUsers.contains(strUserName))
		{
			return false;
		}
		return m_hashUsers[strUserName].compare(strPassword, Qt::CaseSensitive) == 0;
	};
	// Create "Objects" folder using special constructor
	// Part 5 - 8.2.4 : Objects
	auto objectsNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	this->m_newNodeNodeId = &objectsNodeId;
	this->m_newNodeMetaObject = &QUaFolderObject::staticMetaObject;
	m_pobjectsFolder = new QUaFolderObject(this);
	m_pobjectsFolder->setParent(this);
	m_pobjectsFolder->setObjectName( QStringLiteral("Objects") );
	// register base types (for all types)
	this->registerSpecificationType<QUaBaseVariable>    (UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE    ), true);
	this->registerSpecificationType<QUaBaseDataVariable>(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE));
	this->registerSpecificationType<QUaProperty>        (UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE        ));
	this->registerSpecificationType<QUaBaseObject>      (UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE      ));
	this->registerSpecificationType<QUaFolderObject>    (UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE          ));
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	this->registerSpecificationType<QUaBaseEvent              >(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE), true);
	this->registerSpecificationType<QUaBaseModelChangeEvent   >(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEMODELCHANGEEVENTTYPE));
	this->registerSpecificationType<QUaGeneralModelChangeEvent>(UA_NODEID_NUMERIC(0, UA_NS0ID_GENERALMODELCHANGEEVENTTYPE));
	this->registerSpecificationType<QUaSystemEvent            >(UA_NODEID_NUMERIC(0, UA_NS0ID_SYSTEMEVENTTYPE));
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	this->registerSpecificationType<QUaConditionVariable         >(UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONVARIABLETYPE  ));
	this->registerSpecificationType<QUaStateVariable             >(UA_NODEID_NUMERIC(0, UA_NS0ID_STATEVARIABLETYPE      ));
	this->registerSpecificationType<QUaTwoStateVariable          >(UA_NODEID_NUMERIC(0, UA_NS0ID_TWOSTATEVARIABLETYPE   ));
	this->registerSpecificationType<QUaFiniteStateVariable       >(UA_NODEID_NUMERIC(0, UA_NS0ID_FINITESTATEVARIABLETYPE));
	this->registerSpecificationType<QUaTransitionVariable        >(UA_NODEID_NUMERIC(0, UA_NS0ID_TRANSITIONVARIABLETYPE ));
	this->registerSpecificationType<QUaFiniteTransitionVariable  >(UA_NODEID_NUMERIC(0, UA_NS0ID_FINITETRANSITIONVARIABLETYPE));
	this->registerSpecificationType<QUaCondition                 >(UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE), true);
	this->registerSpecificationType<QUaAcknowledgeableCondition  >(UA_NODEID_NUMERIC(0, UA_NS0ID_ACKNOWLEDGEABLECONDITIONTYPE));
	this->registerSpecificationType<QUaAlarmCondition            >(UA_NODEID_NUMERIC(0, UA_NS0ID_ALARMCONDITIONTYPE    ));
	this->registerSpecificationType<QUaStateMachine              >(UA_NODEID_NUMERIC(0, UA_NS0ID_STATEMACHINETYPE      ));
	this->registerSpecificationType<QUaFiniteStateMachine        >(UA_NODEID_NUMERIC(0, UA_NS0ID_FINITESTATEMACHINETYPE), true);
	this->registerSpecificationType<QUaState                     >(UA_NODEID_NUMERIC(0, UA_NS0ID_STATETYPE     ));
	this->registerSpecificationType<QUaTransition                >(UA_NODEID_NUMERIC(0, UA_NS0ID_TRANSITIONTYPE));
	this->registerSpecificationType<QUaExclusiveLimitStateMachine>(UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE));
	this->registerSpecificationType<QUaDiscreteAlarm             >(UA_NODEID_NUMERIC(0, UA_NS0ID_DISCRETEALARMTYPE       ));
	this->registerSpecificationType<QUaOffNormalAlarm            >(UA_NODEID_NUMERIC(0, UA_NS0ID_OFFNORMALALARMTYPE      ));
	this->registerSpecificationType<QUaLimitAlarm                >(UA_NODEID_NUMERIC(0, UA_NS0ID_LIMITALARMTYPE          ));
	this->registerSpecificationType<QUaExclusiveLimitAlarm       >(UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITALARMTYPE ));
	this->registerSpecificationType<QUaExclusiveLevelAlarm       >(UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELEVELALARMTYPE ));
	this->registerSpecificationType<QUaRefreshStartEvent         >(UA_NODEID_NUMERIC(0, UA_NS0ID_REFRESHSTARTEVENTTYPE   ));
	this->registerSpecificationType<QUaRefreshEndEvent           >(UA_NODEID_NUMERIC(0, UA_NS0ID_REFRESHENDEVENTTYPE     ));
	this->registerSpecificationType<QUaRefreshRequiredEvent      >(UA_NODEID_NUMERIC(0, UA_NS0ID_REFRESHREQUIREDEVENTTYPE));
	this->registerSpecificationType<QUaTransitionEvent           >(UA_NODEID_NUMERIC(0, UA_NS0ID_TRANSITIONEVENTTYPE     ));
	// register static condition refresh methods
	// Part 9 - 5.57 and 5.5.8
	st = UA_Server_setMethodNode_callback(
		m_server, 
		UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE_CONDITIONREFRESH), 
		&QUaCondition::ConditionRefresh
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setMethodNode_callback(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE_CONDITIONREFRESH2),
		&QUaCondition::ConditionRefresh2
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	this->registerSpecificationType<QUaOptionSetVariable        >(UA_NODEID_NUMERIC(0, UA_NS0ID_OPTIONSETTYPE));
#endif
	// set context for server
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// add default supported references
	m_hashHierRefTypes.insert({ QStringLiteral("Organizes")          , QStringLiteral("OrganizedBy")        }, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES          ));
	m_hashHierRefTypes.insert({ QStringLiteral("HasOrderedComponent"), QStringLiteral("OrderedComponentOf") }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT));
	m_hashHierRefTypes.insert({ QStringLiteral("HasComponent")       , QStringLiteral("ComponentOf")        }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT       ));
	m_hashHierRefTypes.insert({ QStringLiteral("HasProperty")        , QStringLiteral("PropertyOf")         }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY        ));
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	m_hashHierRefTypes.insert({ QStringLiteral("HasEventSource")     , QStringLiteral("EventSourceOf")      }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASEVENTSOURCE));
	m_hashHierRefTypes.insert({ QStringLiteral("HasNotifier")        , QStringLiteral("NotifierOf")         }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASNOTIFIER   ));
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	// hierarchical
	m_hashHierRefTypes.insert({ QStringLiteral("HasTrueSubState") , QStringLiteral("IsTrueSubStateOf")  }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASTRUESUBSTATE ));
	m_hashHierRefTypes.insert({ QStringLiteral("HasFalseSubState"), QStringLiteral("IsFalseSubStateOf") }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASFALSESUBSTATE));
	// non-hierarchical
	m_hashRefTypes.insert({ QStringLiteral("HasCondition")      , QStringLiteral("IsConditionOf")     }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCONDITION));
	m_hashRefTypes.insert({ QStringLiteral("FromState")         , QStringLiteral("ToTransition")      }, UA_NODEID_NUMERIC(0, UA_NS0ID_FROMSTATE)); // Part 5 - B.4.11
	m_hashRefTypes.insert({ QStringLiteral("ToState")           , QStringLiteral("FromTransition")    }, UA_NODEID_NUMERIC(0, UA_NS0ID_TOSTATE)); // Part 5 - B.4.12
	m_hashRefTypes.insert({ QStringLiteral("HasCause")          , QStringLiteral("MayBeCausedBy")     }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCAUSE)); // Part 5 - B.4.13
	m_hashRefTypes.insert({ QStringLiteral("HasEffect")         , QStringLiteral("MayBeEffectedBy")   }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASEFFECT)); // Part 5 - B.4.14
	m_hashRefTypes.insert({ QStringLiteral("HasSubStateMachine"), QStringLiteral("SubStateMachineOf") }, UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBSTATEMACHINE)); // Part 5 - B.4.15
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	for (auto it = m_hashHierRefTypes.constBegin(); it != m_hashHierRefTypes.constEnd(); ++it)
		m_hashRefTypes.insert(it.key(), it.value());

	// read initial values for server description
	UA_ServerConfig* config = UA_Server_getConfig(m_server);
	// avoid startup warnings with the default config
	config->logger.log=UA_Log_Discard_log;
	UA_ServerConfig_setMinimal(config, m_port, nullptr);
	// setup logger
	config->logger = this->getLogger();
	// get server app name
	m_byteApplicationName = QByteArray(
		(char*)config->applicationDescription.applicationName.text.data,
		(int)config->applicationDescription.applicationName.text.length
	);
	// get server app uri
	m_byteApplicationUri = QByteArray(
		(char*)config->applicationDescription.applicationUri.data,
		(int)config->applicationDescription.applicationUri.length
	);
	// get server product name
	m_byteProductName = QByteArray(
		(char*)config->buildInfo.productName.data,
		(int)config->buildInfo.productName.length
	);
	// get server product uri
	m_byteProductUri = QByteArray(
		(char*)config->buildInfo.productUri.data,
		(int)config->buildInfo.productUri.length
	);
	// get server manufacturer name
	m_byteManufacturerName = QByteArray(
		(char*)config->buildInfo.manufacturerName.data,
		(int)config->buildInfo.manufacturerName.length
	);
	// get server software version
	m_byteSoftwareVersion = QByteArray(
		(char*)config->buildInfo.softwareVersion.data,
		(int)config->buildInfo.softwareVersion.length
	);
	// get server software version
	m_byteBuildNumber = QByteArray(
		(char*)config->buildInfo.buildNumber.data,
		(int)config->buildInfo.buildNumber.length
	);

	// copy other initial values
	m_maxSecureChannels = config->maxSecureChannels;
	m_maxSessions = config->maxSessions;

	// instantiate change event
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	m_changeEvent = this->createEvent<QUaGeneralModelChangeEvent>();
	Q_CHECK_PTR(m_changeEvent);
	m_changeEvent->setSourceNode(QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)));
	m_changeEvent->setSourceName(tr("Server"));
	m_changeEvent->setMessage(tr("Node added or removed."));
	m_changeEvent->setSeverity(1);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	m_refreshStartEvent = this->createEvent<QUaRefreshStartEvent>();
	Q_CHECK_PTR(m_refreshStartEvent);
	m_refreshStartEvent->setSourceNode(QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)));
	m_refreshStartEvent->setSourceName(tr("Server"));
	m_refreshStartEvent->setMessage("");
	m_refreshStartEvent->setSeverity(100);
	m_refreshEndEvent = this->createEvent<QUaRefreshEndEvent>();
	Q_CHECK_PTR(m_refreshEndEvent);
	m_refreshEndEvent->setSourceNode(QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)));
	m_refreshEndEvent->setSourceName(tr("Server"));
	m_refreshEndEvent->setMessage("");
	m_refreshEndEvent->setSeverity(100);
	m_refreshRequiredEvent = this->createEvent<QUaRefreshRequiredEvent>();
	Q_CHECK_PTR(m_refreshRequiredEvent);
	m_refreshRequiredEvent->setSourceNode(QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)));
	m_refreshRequiredEvent->setSourceName(tr("Server"));
	m_refreshRequiredEvent->setMessage("");
	m_refreshRequiredEvent->setSeverity(100);
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#ifdef UA_ENABLE_HISTORIZING
	UA_HistoryDataGathering gathering = UA_HistoryDataGathering_Default(1000);
	m_historDatabase = UA_HistoryDatabase_default(gathering);
	// add historic event handling is supported
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// NOTE : changed setEvent for optimized call in QUaServer_Anex::UA_Server_triggerEvent_Modified
	m_historDatabase.setEvent  = nullptr; 
	m_historDatabase.readEvent = &QUaHistoryBackend::readEvent;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	config->historyDatabase = m_historDatabase;
#endif // UA_ENABLE_HISTORIZING
}

UA_Logger QUaServer::getLogger()
{
	return {
		[](void* logContext, UA_LogLevel level, UA_LogCategory category, const char* msg, va_list args)
		{
#ifdef QT_DEBUG 
			auto srv = qobject_cast<QUaServer*>(static_cast<QObject*>(logContext));
			Q_CHECK_PTR(srv);
#else
			auto srv = static_cast<QUaServer*>(logContext);
#endif // QT_DEBUG 
			// do not process log message if nobody listening
			static const QMetaMethod logSignal = QMetaMethod::fromSignal(&QUaServer::logMessage);
			if (!srv->isSignalConnected(logSignal))
			{
				return;
			}
			vsprintf(srv->m_logBuffer, msg, args);
			emit srv->logMessage({
				QByteArray( srv->m_logBuffer ),
				static_cast<QUaLogLevel>(level),
				static_cast<QUaLogCategory>(category)
			});
		},
		this,
		nullptr
	};
}

QUaServer::~QUaServer()
{
	m_beingDestroyed = true;
	// stop if running
	this->stop();
	// [FIX] : QObject children destructors were called after this one
	//         and the ~QUaNode destructor makes use of m_server
	//         so we better destroy the children manually before deleting m_server
	while (this->children().count() > 0)
	{
		delete this->children().at(0);
	}
	// cleanup open62541
	UA_Server_delete(this->m_server);
}

quint16 QUaServer::port() const
{
	return m_port;
}

void QUaServer::setPort(const quint16& intPort)
{
	m_port = intPort;
	emit this->portChanged(m_port);
}

QByteArray QUaServer::certificate() const
{
	return m_byteCertificate;
}

void QUaServer::setCertificate(const QByteArray& byteCertificate)
{
	m_byteCertificate = byteCertificate;
	emit this->certificateChanged(m_byteCertificate);
}

#ifdef UA_ENABLE_ENCRYPTION
QByteArray QUaServer::privateKey() const
{
	return m_bytePrivateKey;
}

void QUaServer::setPrivateKey(const QByteArray& bytePrivateKey)
{
	m_bytePrivateKey = bytePrivateKey;
	emit this->privateKeyChanged(m_bytePrivateKey);
}
#endif

QString QUaServer::applicationName() const
{
	return QString(m_byteApplicationName);
}

void QUaServer::setApplicationName(const QString& strApplicationName)
{
	// update config
	m_byteApplicationName = strApplicationName.toUtf8();
	// emit event
	emit this->applicationNameChanged(strApplicationName);
}

QString QUaServer::applicationUri() const
{
	return QString(m_byteApplicationUri);
}

void QUaServer::setApplicationUri(const QString& strApplicationUri)
{
	// update config
	m_byteApplicationUri = strApplicationUri.toUtf8();
	// emit event
	emit this->applicationUriChanged(strApplicationUri);
}

QString QUaServer::productName() const
{
	return QString(m_byteProductName);
}

void QUaServer::setProductName(const QString& strProductName)
{
	// update config
	m_byteProductName = strProductName.toUtf8();
	// emit event
	emit this->productNameChanged(strProductName);
}

QString QUaServer::productUri() const
{
	return QString(m_byteProductUri);
}

void QUaServer::setProductUri(const QString& strProductUri)
{
	// update config
	m_byteProductUri = strProductUri.toUtf8();
	// emit event
	emit this->productUriChanged(strProductUri);
}

QString QUaServer::manufacturerName() const
{
	return QString(m_byteManufacturerName);
}

void QUaServer::setManufacturerName(const QString& strManufacturerName)
{
	// update config
	m_byteManufacturerName = strManufacturerName.toUtf8();
	// emit event
	emit this->manufacturerNameChanged(strManufacturerName);
}

QString QUaServer::softwareVersion() const
{
	return QString(m_byteSoftwareVersion);
}

void QUaServer::setSoftwareVersion(const QString& strSoftwareVersion)
{
	// update config
	m_byteSoftwareVersion = strSoftwareVersion.toUtf8();
	// emit event
	emit this->softwareVersionChanged(strSoftwareVersion);
}

QString QUaServer::buildNumber() const
{
	return QString(m_byteBuildNumber);
}

void QUaServer::setBuildNumber(const QString& strBuildNumber)
{
	// update config
	m_byteBuildNumber = strBuildNumber.toUtf8();
	// emit event
	emit this->buildNumberChanged(strBuildNumber);
}

bool QUaServer::start()
{
	// NOTE : we must define port and other server params upon instantiation, 
	//        because rest of API assumes m_server is valid
	if (m_running)
	{
		return true;
	}
	// reset config before starting
	this->resetConfig();
	// start open62541 server
	auto st = UA_Server_run_startup(m_server);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	if (st != UA_STATUSCODE_GOOD)
	{
		return false;
	}
	m_running = true;
	QObject::connect(&m_iterWaitTimer, &QTimer::timeout, this,
	[this]() {
		// do not iterate if asked to stop
		if (!m_running) { return; }
		// iterate and restart
		m_iterWaitTimer.stop();
		// NOTE : any other delay or not waitInternal make subscribing to
		//        events painfully slow
		UA_Server_run_iterate(m_server, true);
		m_iterWaitTimer.start(0);
	}, Qt::QueuedConnection);
	// start iterations
	m_iterWaitTimer.start(0);
	// emit event
	emit this->isRunningChanged(m_running);
	return true;
}

static void
serverExecuteRepeatedCallback(UA_Server* server, UA_ApplicationCallback cb,
	void* callbackApplication, void* data) {
	Q_UNUSED(server);
	/* Service mutex is not set inside the timer that triggers the callback */
	UA_LOCK_ASSERT(server->serviceMutex, 0);
	cb(callbackApplication, data);
}

void QUaServer::stop()
{
	if (!m_running)
	{
		return;
	}
	m_running = false;
	m_iterWaitTimer.stop();
	m_iterWaitTimer.disconnect();
	UA_Server_run_shutdown(m_server);
	// [FIX] force remove channels and sessions
	// NOTE : cannot use UA_Server_cleanup because it only removes timedout sessions
	auto nowMonotonic = (std::numeric_limits<int64_t>::max)();
	UA_Server_cleanupSessions(m_server, nowMonotonic);
	UA_Server_cleanupTimedOutSecureChannels(m_server, nowMonotonic);
#ifdef UA_ENABLE_DISCOVERY
	UA_Discovery_cleanupTimedOut(m_server, nowMonotonic);
#endif
	// NOTE : need to clean delayed callbacks or they will try to cleanup timed out
	// channels and sessions upon restart resulting in crash
	// ??? UA_WorkQueue_manuallyProcessDelayed(&m_server->workQueue);

	// from void UA_Server_delete(UA_Server *server
	/* Execute all remaining delayed events and clean up the timer */
	UA_Timer_process(&m_server->timer, UA_DateTime_nowMonotonic() + 1,
		(UA_TimerExecutionCallback)serverExecuteRepeatedCallback, m_server);

	// emit event
	emit this->isRunningChanged(m_running);
}

bool QUaServer::isRunning() const
{
	return m_running;
}

void QUaServer::setIsRunning(const bool& running)
{
	if (running)
	{
		this->start();
	}
	else
	{
		this->stop();
	}
}

quint16 QUaServer::maxSecureChannels() const
{
	return m_maxSecureChannels;
}

void QUaServer::setMaxSecureChannels(const quint16& maxSecureChannels)
{
	m_maxSecureChannels = maxSecureChannels;
	emit this->maxSecureChannelsChanged(m_maxSecureChannels);
}

quint16 QUaServer::maxSessions() const
{
	return m_maxSessions;
}

void QUaServer::setMaxSessions(const quint16& maxSessions)
{
	m_maxSessions = maxSessions;
	emit this->maxSessionsChanged(m_maxSessions);
}

void QUaServer::setChildNodeIdCallback(const QUaChildNodeIdCallback& callback)
{
	m_childNodeIdCallback = callback;
}

void QUaServer::registerTypeInternal(
	const QMetaObject& metaObject, 
	const QUaNodeId& nodeId/* = ""*/
)
{
	// check if OPC UA relevant
	if (!metaObject.inherits(&QUaNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::registerType", "Unsupported base class");
		return;
	}
	// check if already registered
	QString   strClassName = QString(metaObject.className());
	UA_NodeId newTypeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (!UA_NodeId_isNull(&newTypeNodeId))
	{
		Q_ASSERT(m_mapTypes.contains(strClassName));
		return;
	}
	// create new type browse name
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 0;
	browseName.name = QUaTypesConverter::uaStringFromQString(strClassName);
	// check if base class is registered
	QString strBaseClassName = QString(metaObject.superClass()->className());
	if (!m_mapTypes.contains(strBaseClassName))
	{
		// recursive
		this->registerTypeInternal(*metaObject.superClass());
	}
	Q_ASSERT_X(m_mapTypes.contains(strBaseClassName), "QUaServer::registerType", "Base object type not registered.");
	// check if requested node id defined
	if (!nodeId.isNull())
	{
		// check if requested node id exists
		bool isUsed = this->isNodeIdUsed(nodeId);
		Q_ASSERT_X(!isUsed, "QUaServer::registerType", "Requested NodeId already exists");
		if (isUsed)
		{
			UA_QualifiedName_clear(&browseName);
			return;
		}
	}
	UA_NodeId reqNodeId = nodeId;
	// check if variable or object
	if (metaObject.inherits(&QUaBaseDataVariable::staticMetaObject))
	{
		// create variable type attributes
		UA_VariableTypeAttributes vtAttr = UA_VariableTypeAttributes_default;
		// set node attributes		  
		QByteArray byteDisplayName = strClassName.toUtf8();
		vtAttr.displayName = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
		QByteArray byteDescription;
		vtAttr.description = UA_LOCALIZEDTEXT((char*)"", byteDescription.data());
		// add new variable type
		auto st = UA_Server_addVariableTypeNode(m_server,
			reqNodeId,                                 // requested nodeId
			m_mapTypes.value(strBaseClassName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE)), // parent (variable type)
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), // parent relation with child
			browseName,
			UA_NODEID_NULL,                            // typeDefinition ??
			vtAttr,
			(void*)this,                               // context : server instance where type was registered
			&newTypeNodeId);                           // new variable type id
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	else
	{
		Q_ASSERT(metaObject.inherits(&QUaBaseObject::staticMetaObject));
		// create object type attributes
		UA_ObjectTypeAttributes otAttr = UA_ObjectTypeAttributes_default;
		// set node attributes		  
		QByteArray byteDisplayName = strClassName.toUtf8();
		otAttr.displayName = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
		QByteArray byteDescription;
		otAttr.description = UA_LOCALIZEDTEXT((char*)"", byteDescription.data());
		// add new object type
		auto st = UA_Server_addObjectTypeNode(m_server,
			reqNodeId,                                 // requested nodeId
			m_mapTypes.value(strBaseClassName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)), // parent (object type)
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), // parent relation with child
			browseName,
			otAttr,
			(void*)this,                               // context : server instance where type was registered
			&newTypeNodeId);                           // new object type id
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	// clean up
	UA_QualifiedName_clear(&browseName);
	UA_NodeId_clear(&reqNodeId);
	// add to registered types map
	m_mapTypes.insert(strClassName, newTypeNodeId);
	m_hashMetaObjects.insert(strClassName, metaObject);
	// register for default mandatory children and so on
	// in case our custom type inherits from a spec type
	// which contains mandatory children
	this->registerTypeDefaults(newTypeNodeId, metaObject);
	// register constructor/destructor
	this->registerTypeLifeCycle(newTypeNodeId, metaObject);
	// register meta-enums
	this->registerMetaEnums(metaObject);
	// register meta-properties
	// NOTE : this can be recursive if property type has not been yet registered
	this->addMetaProperties(metaObject);
	// register meta-methods (only if object class, or NOT variable class)
	if (!metaObject.inherits(&QUaBaseDataVariable::staticMetaObject))
	{
		this->addMetaMethods(metaObject);
	}
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// needed to store historic events in a consistent way
	if (metaObject.inherits(&QUaBaseEvent::staticMetaObject))
	{
		Q_ASSERT(!m_hashTypeVars.contains(newTypeNodeId));
		m_hashTypeVars[newTypeNodeId] =
			QUaNode::getTypeVars(
				newTypeNodeId,
				this->m_server
			);
	}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
}

QList<QUaNode*> QUaServer::typeInstances(const QMetaObject& metaObject)
{
	QList<QUaNode*> retList;
	// check if OPC UA relevant
	if (!metaObject.inherits(&QUaNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::typeInstances", "Unsupported base class. It must derive from QUaNode");
		return retList;
	}
	// try to get typeNodeId, if null, then register it
	UA_NodeId typeNodeId = this->typeIdByMetaObject(metaObject);
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));
	// make ua browse
	auto refTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
	QList<UA_NodeId> retRefSet;
	UA_BrowseDescription* bDesc = UA_BrowseDescription_new();
	UA_NodeId_copy(&typeNodeId, &bDesc->nodeId);
	bDesc->browseDirection = UA_BROWSEDIRECTION_INVERSE; //TypeDefinitionOf
	bDesc->includeSubtypes = true;
	bDesc->referenceTypeId = refTypeId;
	bDesc->resultMask = UA_BROWSERESULTMASK_REFERENCETYPEID;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(m_server, 0, bDesc);
	Q_ASSERT(bRes.statusCode == UA_STATUSCODE_GOOD);
	while (bRes.referencesSize > 0)
	{
		for (size_t i = 0; i < bRes.referencesSize; i++)
		{
			UA_ReferenceDescription rDesc = bRes.references[i];
			Q_ASSERT(UA_NodeId_equal(&rDesc.referenceTypeId, &refTypeId));
			UA_NodeId nodeId/* = rDesc.nodeId.nodeId*/;
			UA_NodeId_copy(&rDesc.nodeId.nodeId, &nodeId);
			retRefSet << nodeId;
		}
        UA_BrowseResult_clear(&bRes);
		bRes = UA_Server_browseNext(m_server, true, &bRes.continuationPoint);
	}
	// cleanup
    UA_BrowseDescription_clear(bDesc);
	UA_BrowseDescription_delete(bDesc);
    UA_BrowseResult_clear(&bRes);
	// get QUaNode references
	for (int i = 0; i < retRefSet.count(); i++)
	{
		// when browsing ObjectsFolder there are children with null context (Server object and children)
		QUaNode* node = QUaNode::getNodeContext(retRefSet[i], m_server);
		if (node)
		{
			retList << node;
		}
		UA_NodeId_clear(&retRefSet[i]);
	}
	return retList;
}

void QUaServer::registerTypeLifeCycle(const UA_NodeId& typeNodeId, const QMetaObject& metaObject)
{
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));
	if (UA_NodeId_isNull(&typeNodeId))
	{
		return;
	}
	// add custom constructor
	Q_ASSERT_X(!m_hashConstructors.contains(typeNodeId), "QUaServer::registerType", "Constructor for type already exists.");
	// NOTE : we need constructors to be lambdas in order to cache metaobject in capture
	//        because type context is already the server instance where type was registered
	//        so we can differentiate the server instance in the static ::uaConstructor callback
	//        TLDR; to support multiple server instances in an application
	m_hashConstructors[typeNodeId] = [metaObject, this](const UA_NodeId* instanceNodeId, void** nodeContext) {
		// call static method
		return QUaServer::uaConstructor(this, instanceNodeId, nodeContext, metaObject);
	};
	// set generic constructor (that calls custom one internally)
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &QUaServer::uaConstructor;
	lifecycle.destructor = &QUaServer::uaDestructor;
	auto st = UA_Server_setNodeTypeLifecycle(m_server, typeNodeId, lifecycle);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st)
}

void QUaServer::registerTypeDefaults(const UA_NodeId& typeNodeId, const QMetaObject& metaObject)
{
	// cache mandatory children if not done before
	Q_ASSERT(!m_hashMandatoryChildren.contains(typeNodeId));
	if (m_hashMandatoryChildren.contains(typeNodeId))
	{
		return;
	}
	// copy mandatory from parent type, unless base type
    static UA_NodeId baseObjType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
    static UA_NodeId baseVarType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE);
    if (UA_NodeId_equal(&typeNodeId, &baseVarType) ||
        UA_NodeId_equal(&typeNodeId, &baseObjType))
	{
		m_hashMandatoryChildren[typeNodeId] = QSet<QUaQualifiedName>();
		return;
	}
	QUaNodeId superTypeNodeId = QUaNode::superTypeDefinitionNodeId(typeNodeId, this->m_server);
	Q_ASSERT_X(
		m_hashMandatoryChildren.contains(superTypeNodeId) ||
		metaObject.superClass() == &QUaNode::staticMetaObject,
		"QUaServer::registerTypeDefaults", "Parent type must already be registered.");
	m_hashMandatoryChildren[typeNodeId] =
		m_hashMandatoryChildren.value(superTypeNodeId, QSet<QUaQualifiedName>());
	// get mandatory children browse names
	auto chidrenNodeIds = QUaNode::getChildrenNodeIds(typeNodeId, m_server);
	for (const auto & childNodeId : chidrenNodeIds)
	{
		// ignore if not mandatory
		if (!QUaNode::hasMandatoryModellingRule(childNodeId, m_server))
		{
			continue;
		}
		// sometimes children repeat parent's mandatory, no need to add twice
		QUaQualifiedName mandatoryBrowseName = QUaNode::getBrowseName(childNodeId, m_server);
		if (m_hashMandatoryChildren[typeNodeId].contains(mandatoryBrowseName))
		{
			continue;
		}
		m_hashMandatoryChildren[typeNodeId]
			<< mandatoryBrowseName;
	}
	// get qt type methods by name
	// loop meta methods and find out which ones inherit from
	QHash<QUaQualifiedName, int> hashQtMethods = QUaServer::metaMethodIndexes(metaObject);
	// get all ua methods
	auto methodsNodeIds = QUaNode::getMethodsNodeIds(typeNodeId, m_server);
	// try to match ua methods with qt meta methods (by browse name)
	for (const auto & methodNodeId : methodsNodeIds)
	{
		// ignore if not mandatory or optional
		if (!QUaNode::hasMandatoryModellingRule(methodNodeId, m_server) &&
			!QUaNode::hasOptionalModellingRule(methodNodeId, m_server))
		{
			continue;
		}
		// check mandatory ua method exists on qt type
		QUaQualifiedName methBrowseName = QUaNode::getBrowseName(methodNodeId, m_server);
		Q_ASSERT_X(hashQtMethods.contains(methBrowseName), "QUaServer::registerTypeDefaults",
			"Qt type does not implement method. Qt types must implement both mandatory and optional.");
		if (!hashQtMethods.contains(methBrowseName))
		{
			QString strClassName = QString(metaObject.className());
			qDebug() << "Error Registering" << methBrowseName << "for" << strClassName;
			continue;
		}

		// TODO : check arguments and return types

		int methIdx = hashQtMethods[methBrowseName];
		QUaServer::bindMethod(this, &methodNodeId, methIdx);
	}
	// cleanup for all ua methods
	for (auto & methNodeId : methodsNodeIds)
	{
		UA_NodeId_clear(&methNodeId);
	}
}

void QUaServer::registerMetaEnums(const QMetaObject& metaObject)
{
	int enumCount = metaObject.enumeratorCount();
	for (int i = metaObject.enumeratorOffset(); i < enumCount; i++)
	{
		QMetaEnum metaEnum = metaObject.enumerator(i);
		this->registerEnum(metaEnum);
	}
}

void QUaServer::addMetaProperties(const QMetaObject& metaObject)
{
	UA_NodeId parentTypeNodeId = this->typeIdByMetaObject(metaObject);
	Q_ASSERT(!UA_NodeId_isNull(&parentTypeNodeId));
	// loop meta properties and find out which ones inherit from
	int propCount = metaObject.propertyCount();
	for (int i = metaObject.propertyOffset(); i < propCount; i++)
	{
		QMetaProperty metaProperty = metaObject.property(i);
		// check if is meta enum
		bool      isVariable = false;
		bool      isEnum = false;
		UA_NodeId enumTypeNodeId = UA_NODEID_NULL;
		if (metaProperty.isEnumType())
		{
			QMetaEnum metaEnum = metaProperty.enumerator();
			// compose enum name
#if QT_VERSION >= 0x051200
			QString strEnumName = QStringLiteral("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
#else
			QString strEnumName = QStringLiteral("%1::%2").arg(metaEnum.scope()).arg(metaEnum.name());
#endif
			// must already be registered by now
			Q_ASSERT(m_hashEnums.contains(strEnumName));
			// get enum data type
			enumTypeNodeId = m_hashEnums.value(strEnumName);
			// allow to continue
			isEnum = true;
		}
		// parent relation with child
		UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
		// get property name
		QByteArray bytePropName = metaProperty.name();
		// get type node id
		UA_NodeId propTypeNodeId;
		if (!isEnum)
		{
			// check if available in meta-system
			const QMetaObject *propMetaObject = QMetaType(metaProperty.userType()).metaObject();
			if (!propMetaObject)
			{
				continue;
			}
			// check if OPC UA relevant type
			if (!propMetaObject->inherits(&QUaNode::staticMetaObject))
			{
				continue;
			}
			// check if prop inherits from parent
			Q_ASSERT_X(!propMetaObject->inherits(&metaObject),
				"QUaServer::addMetaProperties",
				"Qt MetaProperty type cannot inherit from Class.");
			if (propMetaObject->inherits(&metaObject) && !isEnum)
			{
				continue;
			}
			// check if prop type registered, register of not
			propTypeNodeId = this->typeIdByMetaObject(*propMetaObject);
			// set is variable
			isVariable = propMetaObject->inherits(&QUaBaseVariable::staticMetaObject);
			// check if ua property, then set correct reference
			if (propMetaObject->inherits(&QUaProperty::staticMetaObject))
			{
				referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
			}
		}
		else
		{
			propTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
		}
		Q_ASSERT(!UA_NodeId_isNull(&propTypeNodeId));
		// set qualified name, default is class name
		UA_QualifiedName browseName;
		// NOTE : use namespace 0 as default to allow QUaNode::browseChild 
		//        to work with simple strings on custom types
		browseName.namespaceIndex = 0;
		browseName.name = QUaTypesConverter::uaStringFromQString( QString::fromLatin1( bytePropName ) );
		// display name
		UA_LocalizedText displayName = UA_LOCALIZEDTEXT((char*)"", bytePropName.data());
		// check if variable or object
		// NOTE : a type is considered to inherit itself sometimes does not work 
		// (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
		UA_NodeId tempNodeId;
		if (isVariable || isEnum)
		{

			// some types require the attrs to match because open62541 checks them
			UA_VariableAttributes vAttr = UA_VariableAttributes_default;
			auto st = UA_Server_readDataType(m_server, propTypeNodeId, &vAttr.dataType);
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
			st = UA_Server_readValueRank(m_server, propTypeNodeId, &vAttr.valueRank);
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
			UA_Variant outArrayDimensions;
			st = UA_Server_readArrayDimensions(m_server, propTypeNodeId, &outArrayDimensions);
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
			vAttr.arrayDimensionsSize = outArrayDimensions.arrayLength;
			vAttr.arrayDimensions = static_cast<quint32*>(outArrayDimensions.data);
			// browse name
			vAttr.displayName = displayName;
			// if enum, set data type
			if (isEnum)
			{
				Q_ASSERT(!UA_NodeId_isNull(&enumTypeNodeId));
				vAttr.dataType = enumTypeNodeId;
			}
			// add variable
			st = UA_Server_addVariableNode(
				m_server,
				UA_NODEID_NULL,   // requested nodeId
				parentTypeNodeId, // parent
				referenceTypeId,  // parent relation with child
				browseName,
				propTypeNodeId,
				vAttr,
				nullptr,          // context
				&tempNodeId       // output nodeId to make mandatory
			);
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
			UA_Variant_clear(&outArrayDimensions);
		}
		else
		{
			// NOTE : not working ! 
			// Q_ASSERT(propMetaObject.inherits(&QUaBaseObject::staticMetaObject));
			UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
			oAttr.displayName = displayName;
			// add object
			auto st = UA_Server_addObjectNode(
				m_server,
				UA_NODEID_NULL,   // requested nodeId
				parentTypeNodeId, // parent
				referenceTypeId,  // parent relation with child
				browseName,
				propTypeNodeId,
				oAttr,
				nullptr,          // context
				&tempNodeId);     // output nodeId to make mandatory
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
		}
		// clean up
		UA_QualifiedName_clear(&browseName);
		// make mandatory
		auto st = UA_Server_addReference(m_server,
			tempNodeId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
			UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY),
			true);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
}

void QUaServer::addMetaMethods(const QMetaObject& parentMetaObject)
{
	UA_NodeId parentTypeNodeId = this->typeIdByMetaObject(parentMetaObject);
	Q_ASSERT(!UA_NodeId_isNull(&parentTypeNodeId));
	// loop meta methods and find out which ones inherit from
	for (int methIdx = parentMetaObject.methodOffset(); methIdx < parentMetaObject.methodCount(); methIdx++)
	{
		QMetaMethod metaMethod = parentMetaObject.method(methIdx);
		// validate id method (not signal, slot or constructor)
		auto methodType = metaMethod.methodType();
		if (methodType != QMetaMethod::Method)
		{
			continue;
		}
		// validate return type
		auto returnType = (QMetaType::Type)metaMethod.returnType();
		bool isSupported = QUaTypesConverter::isSupportedQType(returnType);
		bool isEnumType = this->m_hashEnums.contains(metaMethod.typeName());
		bool isArrayType = QUaTypesConverter::isQTypeArray(returnType);
		bool isValidType = isSupported || isEnumType || isArrayType;
		// NOTE : enums are QMetaType::UnknownType
		Q_ASSERT_X(isValidType,
			"QUaServer::addMetaMethods",
			"Return type not supported in MetaMethod.");
		if (!isValidType)
		{
			continue;
		}
		// if array
		if (isArrayType)
		{
			returnType = QUaTypesConverter::getQArrayType(returnType);
		}
		// create return type
		UA_Argument  outputArgumentInstance;
		UA_Argument* outputArgument = nullptr;
		if (returnType != QMetaType::Void)
		{
			UA_Argument_init(&outputArgumentInstance);
			outputArgumentInstance.description = UA_LOCALIZEDTEXT((char*)"",
				(char*)"Result Value");
			outputArgumentInstance.name = QUaTypesConverter::uaStringFromQString( QStringLiteral("Result") );
			outputArgumentInstance.dataType = isEnumType ?
				this->m_hashEnums.value(QString(metaMethod.typeName())) :
				QUaTypesConverter::uaTypeNodeIdFromQType(returnType);
			outputArgumentInstance.valueRank = isArrayType ?
				UA_VALUERANK_ONE_DIMENSION :
				UA_VALUERANK_SCALAR;
			outputArgument = &outputArgumentInstance;
		}
		// validate argument types and create them
		Q_ASSERT_X(metaMethod.parameterCount() <= 10,
			"QUaServer::addMetaMethods",
			"No more than 10 arguments supported in MetaMethod.");
		if (metaMethod.parameterCount() > 10)
		{
			continue;
		}
		QVector<UA_Argument> vectArgs;
		auto listArgNames = metaMethod.parameterNames();
		Q_ASSERT(listArgNames.count() == metaMethod.parameterCount());
		auto listTypeNames = metaMethod.parameterTypes();
		for (int k = 0; k < metaMethod.parameterCount(); k++)
		{
			auto argType = (QMetaType::Type)metaMethod.parameterType(k);
			isSupported = QUaTypesConverter::isSupportedQType(argType);
			isEnumType  = this->m_hashEnums.contains(listTypeNames[k]);
			isArrayType = QUaTypesConverter::isQTypeArray(argType);
			isValidType = isSupported || isEnumType || isArrayType;
			// NOTE : enums are QMetaType::UnknownType
			Q_ASSERT_X(isValidType,
				"QUaServer::addMetaMethods",
				"Argument type not supported in MetaMethod.");
			if (!isValidType)
			{
				break;
			}
			// check if array
			if (isArrayType)
			{
				argType = QUaTypesConverter::getQArrayType(argType);
			}
			// get ua type
			UA_Argument inputArgument;
			UA_Argument_init(&inputArgument);
			// check if type is registered enum
			UA_NodeId uaType = isEnumType ?
				this->m_hashEnums.value(listTypeNames[k]) :
				QUaTypesConverter::uaTypeNodeIdFromQType(argType);
			// create n-th argument
			inputArgument.description = UA_LOCALIZEDTEXT((char*)"", (char*)"Method Argument");
			inputArgument.name = QUaTypesConverter::uaStringFromQString(listArgNames[k]);
			inputArgument.dataType = uaType;
			inputArgument.valueRank = UA_VALUERANK_SCALAR;
			if (isArrayType)
			{
				inputArgument.valueRank = UA_VALUERANK_ONE_DIMENSION;
			}
			vectArgs.append(inputArgument);
		}
		// skip if any arg is not supported
		if (!isValidType)
		{
			continue;
		}
		// add method
		auto strMethName = metaMethod.name();
		// add method node
		UA_MethodAttributes methAttr = UA_MethodAttributes_default;
		methAttr.executable = true;
		methAttr.userExecutable = true;
		methAttr.description = UA_LOCALIZEDTEXT((char*)"",
			strMethName.data());
		methAttr.displayName = UA_LOCALIZEDTEXT((char*)"",
			strMethName.data());
		// create callback
		UA_NodeId methNodeId;
		auto st = UA_Server_addMethodNode(
			this->m_server,
			UA_NODEID_NULL,
			parentTypeNodeId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			UA_QUALIFIEDNAME(1, strMethName.data()),
			methAttr,
			&QUaServer::methodCallback,
			metaMethod.parameterCount(),
			vectArgs.data(),
			outputArgument ? 1 : 0,
			outputArgument,
			this, // context is server instance that has m_hashMethods
			&methNodeId
		);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		// Define "StartPump" method mandatory
		st = UA_Server_addReference(
			this->m_server,
			methNodeId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
			UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY),
			true
		);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		// store method with node id hash as key
		Q_ASSERT_X(!m_hashMethods.contains(methNodeId),
			"QUaServer::addMetaMethods",
			"Method already exists, callback will be overwritten.");
		// NOTE : had to cache the method's index in lambda capture because caching 
		//        metaMethod directly was not working in some cases the internal data
		//        of the metaMethod was deleted which resulted in access violation
		m_hashMethods[methNodeId] = [methIdx, this](
			void* objectContext,
			const UA_Variant* input,
			UA_Variant* output)
		{
			// get object instance that owns method
#ifdef QT_DEBUG 
			QUaBaseObject* object = qobject_cast<QUaBaseObject*>(static_cast<QObject*>(objectContext));
			Q_ASSERT_X(object,
				"QUaServer::addMetaMethods",
				"Cannot call method on invalid C++ object.");
#else
			QUaBaseObject* object = static_cast<QUaBaseObject*>(objectContext);
#endif // QT_DEBUG 
			if (!object)
			{
				return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
			}
			// get meta method
			auto metaMethod = object->metaObject()->method(methIdx);
			return QUaServer::callMetaMethod(this, object, metaMethod, input, output);
		};
	}
}

// NOTE : do not clean
UA_NodeId QUaServer::typeIdByMetaObject(const QMetaObject& metaObject)
{
	QString   strClassName = QString(metaObject.className());
	UA_NodeId typeNodeId   = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (UA_NodeId_isNull(&typeNodeId))
	{
		this->registerTypeInternal(metaObject);
		typeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	}
	return typeNodeId;
}

UA_NodeId QUaServer::createInstanceInternal(
	const QMetaObject& metaObject,
	QUaNode* parentNode,
	const QUaQualifiedName& browseName,
	const QUaNodeId& nodeId
)
{
	// check if OPC UA relevant
	if (!metaObject.inherits(&QUaNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::createInstance",
			"Unsupported base class. It must derive from QUaNode");
		return UA_NODEID_NULL;
	}
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// check if inherits BaseEventType, in which case this method cannot be used
	if (metaObject.inherits(&QUaBaseEvent::staticMetaObject) 
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
		&& !metaObject.inherits(&QUaCondition::staticMetaObject)
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
		)
	{
		Q_ASSERT_X(false, "QUaServer::createInstanceInternal",
			"Cannot use createInstance to create Non-Condition Events. Use createEvent method instead");
		return UA_NODEID_NULL;
	}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// try to get typeNodeId, if null, then register it
	UA_NodeId typeNodeId = this->typeIdByMetaObject(metaObject);
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));
	// adapt parent relation with child according to parent type
	// NOTE : parent can be null (no address space representation, e.g. events, conditions)
	UA_NodeId referenceTypeId = parentNode ?
		QUaServer::getReferenceTypeId(*parentNode->metaObject(), metaObject) :
		UA_NODEID_NULL;
	// check if browse name already used with parent (if any parent)
	if (parentNode && parentNode->hasChild(browseName))
	{
		Q_ASSERT_X(false, "QUaServer::createInstance", "Requested BrowseName already exists in parent");
		return UA_NODEID_NULL;
	}
	UA_QualifiedName uaBrowseName = browseName;
	// check if requested node id defined
	if (!nodeId.isNull())
	{
		// check if requested node id exists
		bool isUsed = this->isNodeIdUsed(nodeId);
		Q_ASSERT_X(!isUsed, "QUaServer::createInstance", "Requested NodeId already exists");
		if (isUsed)
		{
			return UA_NODEID_NULL;
		}
	}
	// NOTE : calling UA_Server_addXXX below will trigger QUaServer::uaConstructor
	// which will instantiate the respective Qt instance and binding
	UA_NodeId reqNodeId = nodeId;
	UA_NodeId nodeIdNewInstance;
	// check if variable or object 
	// NOTE : a type is considered to inherit itself 
	// (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
	Q_ASSERT(parentNode ? !UA_NodeId_isNull(&parentNode->m_nodeId) : true);
	if (metaObject.inherits(&QUaBaseVariable::staticMetaObject))
	{
		// some types require the attrs to match because open62541 checks them
		UA_VariableAttributes vAttr = UA_VariableAttributes_default;
		auto st = UA_Server_readDataType(m_server, typeNodeId, &vAttr.dataType);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		st = UA_Server_readValueRank(m_server, typeNodeId, &vAttr.valueRank);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		UA_Variant outArrayDimensions;
		st = UA_Server_readArrayDimensions(m_server, typeNodeId, &outArrayDimensions);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		vAttr.arrayDimensionsSize = outArrayDimensions.arrayLength;
		vAttr.arrayDimensions = static_cast<quint32*>(outArrayDimensions.data);
		// default displayName is browseName
		QByteArray byteDisplayName = browseName.name().toUtf8();
		vAttr.displayName = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
		// add variable
        st = UA_Server_addVariableNode(m_server,
			reqNodeId,            // requested nodeId
			parentNode ? parentNode->m_nodeId : UA_NODEID_NULL, // parent (can be null)
			referenceTypeId,      // parent relation with child
			uaBrowseName,
			typeNodeId,
			vAttr,
			nullptr,             // context
			&nodeIdNewInstance); // set new nodeId to new instance
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		UA_Variant_clear(&outArrayDimensions);
	}
	else
	{
		Q_ASSERT(metaObject.inherits(&QUaBaseObject::staticMetaObject) ||
			metaObject.className() == QUaBaseObject::staticMetaObject.className());
		UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		// default displayName is browseName
		QByteArray byteDisplayName = browseName.name().toUtf8();
		oAttr.displayName = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
		// add object
		auto st = UA_Server_addObjectNode(m_server,
			reqNodeId,            // requested nodeId
			parentNode ? parentNode->m_nodeId : UA_NODEID_NULL, // parent
			referenceTypeId,      // parent relation with child
			uaBrowseName,
			typeNodeId,
			oAttr,
			nullptr,             // context
			&nodeIdNewInstance); // set new nodeId to new instance
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	Q_ASSERT_X(!UA_NodeId_isNull(&nodeIdNewInstance), "QUaServer::createInstanceInternal", "Something went wrong");
	// clean up
	UA_NodeId_clear(&reqNodeId);
	// NOTE : do not UA_NodeId_clear(&typeNodeId); or value in m_mapTypes gets corrupted
	UA_NodeId_clear(&referenceTypeId);
	UA_QualifiedName_clear(&uaBrowseName);

	// if child is condition, add non-hierarchical reference and default props
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	if (parentNode && metaObject.inherits(&QUaCondition::staticMetaObject))
	{
		// add HasCondition reference
		auto node = QUaNode::getNodeContext(nodeIdNewInstance, this->m_server);
		auto condition = qobject_cast<QUaCondition*>(node);
		Q_CHECK_PTR(condition);
		// set default originator
		condition->QUaBaseEvent::setSourceNode(parentNode);
	}
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	// trigger reference added, model change event, so client (UaExpert) auto refreshes tree
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	if (parentNode && parentNode->inAddressSpace())
	{
		Q_CHECK_PTR(m_changeEvent);
		// add reference added change to buffer
		this->addChange({
			parentNode->nodeId(),
			parentNode->typeDefinitionNodeId(),
			QUaChangeVerb::ReferenceAdded // UaExpert does not recognize QUaChangeVerb::NodeAdded
		});
	}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// return new instance node id
	return nodeIdNewInstance;
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

UA_NodeId QUaServer::createEventInternal(
	const QMetaObject& metaObject
)
{
	// check if derives from event
	if (!metaObject.inherits(&QUaBaseEvent::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::createEvent",
			"Unsupported event class. It must derive from QUaBaseEvent");
		return UA_NODEID_NULL;
	}
	// try to get typeEvtId, if null, then register it
	UA_NodeId typeEvtId = this->typeIdByMetaObject(metaObject);
	Q_ASSERT(!UA_NodeId_isNull(&typeEvtId));
	// create event instance
	UA_NodeId nodeIdNewEvent;
	auto st = UA_Server_createEvent(m_server, typeEvtId, &nodeIdNewEvent);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return new instance node id
	return nodeIdNewEvent;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

void QUaServer::bindCppInstanceWithUaNode(QUaNode* nodeInstance, UA_NodeId& nodeId)
{
	Q_CHECK_PTR(nodeInstance);
	Q_ASSERT(!UA_NodeId_isNull(&nodeId));
	// set c++ instance as context
	UA_Server_setNodeContext(m_server, nodeId, (void**)(&nodeInstance));
	// set node id to c++ instance
	nodeInstance->m_nodeId = nodeId;
}

bool QUaServer::isMetaObjectRegistered(const QString& strClassName) const
{
	return m_hashMetaObjects.contains(strClassName);
}

QMetaObject QUaServer::getRegisteredMetaObject(const QString& strClassName) const
{
	Q_ASSERT_X(m_hashMetaObjects.contains(strClassName), "QUaServer::getRegisteredMetaObject", "Class is not registered.");
	return m_hashMetaObjects.value(strClassName);
}

bool QUaServer::registerReferenceType(const QUaReferenceType& refType, const QUaNodeId& nodeId/* = ""*/)
{
	// first check if already registered
	if (m_hashRefTypes.contains(refType))
	{
		return true;
	}
	// check if requested node id defined
	if (!nodeId.isNull())
	{
		// check if requested node id exists
		bool isUsed = this->isNodeIdUsed(nodeId);
		Q_ASSERT_X(!isUsed, "QUaServer::registerReferenceType", "Requested NodeId already exists");
		if (isUsed)
		{
			return false;
		}
	}
	// get namea and stuff
	QByteArray byteForwardName = refType.strForwardName.toUtf8();
	QByteArray byteInverseName = refType.strInverseName.toUtf8();
	// TODO : Use QUaQualifiedName, but then need to change how refsare serialized
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 0;
	browseName.name = QUaTypesConverter::uaStringFromQString(refType.strForwardName);
	// setup new ref type attributes
	UA_ReferenceTypeAttributes refattr = UA_ReferenceTypeAttributes_default;
	refattr.displayName = UA_LOCALIZEDTEXT((char*)(""), byteForwardName.data());
	refattr.inverseName = UA_LOCALIZEDTEXT((char*)(""), byteInverseName.data());
	UA_NodeId reqNodeId = nodeId;
	UA_NodeId outNewNodeId;
	auto st = UA_Server_addReferenceTypeNode(
		m_server,
		reqNodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_NONHIERARCHICALREFERENCES),
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		browseName,
		refattr,
		nullptr,
		&outNewNodeId
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// clean up
	UA_NodeId_clear(&reqNodeId);
	UA_QualifiedName_clear(&browseName);
	// add to hash to complete registration
	m_hashRefTypes.insert(refType, outNewNodeId);
	return true;
}

const QList<QUaReferenceType> QUaServer::referenceTypes() const
{
	return m_hashRefTypes.keys();
}

bool QUaServer::referenceTypeRegistered(const QUaReferenceType& refType) const
{
	return m_hashRefTypes.contains(refType);
}

bool QUaServer::isNodeIdUsed(const QUaNodeId& nodeId) const
{
	// check if requested node id exists
	UA_NodeId reqNodeId  = nodeId;
	UA_NodeId testNodeId = nodeId;
	auto st = UA_Server_readNodeId(m_server, reqNodeId, &testNodeId);
	UA_NodeId_clear(&reqNodeId);
	UA_NodeId_clear(&testNodeId);
	return st == UA_STATUSCODE_GOOD;
}

QUaFolderObject* QUaServer::objectsFolder() const
{
	return m_pobjectsFolder;
}

QUaNode* QUaServer::nodeById(const QUaNodeId& nodeIdIn)
{
	UA_NodeId nodeId = nodeIdIn;
	QUaNode* node = QUaNode::getNodeContext(nodeId, m_server);
	UA_NodeId_clear(&nodeId);
	return node;
}

bool QUaServer::isTypeNameRegistered(const QString& strTypeName) const
{
	return m_mapTypes.contains(strTypeName);
}

QUaNode * QUaServer::browsePath(const QUaBrowsePath& browsePath) const
{
	if (browsePath.count() <= 0)
	{
		return nullptr;
	}
	QUaQualifiedName first = browsePath.first();
	// check if first is ObjectsFolder
	if (first == this->objectsFolder()->browseName())
	{
		return this->objectsFolder()->browsePath(browsePath.mid(1));
	}
	// then check if first is a child of ObjectsFolder
	auto listChildren = this->objectsFolder()->browseChildren();
	for (auto child : listChildren)
	{
		if (first == child->browseName())
		{
			return child->browsePath(browsePath.mid(1));
		}
	}
	// if not, then not supported
	return nullptr;
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
#ifdef UA_ENABLE_HISTORIZING

bool QUaServer::eventHistoryRead() const
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	return eventNotifier.bits.bHistoryRead;
}

void QUaServer::setEventHistoryRead(const bool& eventHistoryRead)
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	eventNotifier.bits.bHistoryRead = eventHistoryRead;
	this->setEventNotifier(eventNotifier.intValue);
}

quint64 QUaServer::maxHistoryEventResponseSize() const
{
	return m_maxHistoryEventResponseSize;
}

void QUaServer::setMaxHistoryEventResponseSize(const quint64& maxHistoryEventResponseSize)
{
	m_maxHistoryEventResponseSize = maxHistoryEventResponseSize;
}

#endif // UA_ENABLE_HISTORIZING
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

bool QUaServer::anonymousLoginAllowed() const
{
	return m_anonymousLoginAllowed;
}

void QUaServer::setAnonymousLoginAllowed(const bool & anonymousLoginAllowed)
{
	m_anonymousLoginAllowed = anonymousLoginAllowed;
	emit this->anonymousLoginAllowedChanged(m_anonymousLoginAllowed);
}

void QUaServer::addUser(const QString & strUserName, const QString & strKey)
{
	if (strUserName.isEmpty())
	{
		return;
	}
	m_hashUsers[strUserName] = strKey;
}

void QUaServer::removeUser(const QString & strUserName)
{
	if (strUserName.isEmpty())
	{
		return;
	}
	m_hashUsers.remove(strUserName);
}

QString QUaServer::userKey(const QString & strUserName) const
{
	return m_hashUsers.value(strUserName, QString());
}

int QUaServer::userCount()
{
	return m_hashUsers.count();
}

QStringList QUaServer::userNames() const
{
	return m_hashUsers.keys();
}

bool QUaServer::userExists(const QString & strUserName) const
{
	Q_ASSERT(!strUserName.isEmpty());
	if (strUserName.isEmpty())
	{
		return false;
	}
	return m_anonUsers.contains(strUserName) || m_hashUsers.contains(strUserName);
}

QList<const QUaSession*> QUaServer::sessions() const
{
    QList<const QUaSession*> listConstSessions;
    for (auto session : m_hashSessions.values())
    {
        listConstSessions << session;
    }
    return listConstSessions;
}

UA_NodeId QUaServer::getReferenceTypeId(const QMetaObject & parentMetaObject, const QMetaObject & childMetaObject)
{
	UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	// adapt parent relation with child according to parent type
	if (parentMetaObject.inherits(&QUaFolderObject::staticMetaObject))
	{
		referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	}
	else if (parentMetaObject.inherits(&QUaBaseObject::staticMetaObject) ||
		     parentMetaObject.inherits(&QUaBaseDataVariable::staticMetaObject))
	{
		if (childMetaObject.inherits(&QUaBaseObject::staticMetaObject) || 
			childMetaObject.inherits(&QUaBaseDataVariable::staticMetaObject))
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
		}
		else if (childMetaObject.inherits(&QUaProperty::staticMetaObject))
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
		}
	}
	else
	{
		Q_ASSERT_X(false, "QUaServer::getReferenceTypeId", "Invalid parent type.");
	}
	return referenceTypeId;
}
