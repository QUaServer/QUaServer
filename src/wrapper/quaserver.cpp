#include "quaserver.h"

#include <QMetaProperty>
#include <QTimer>

#define QUA_DEBOUNCE_PERIOD_MS 50

// Copied from open62541.c
typedef struct {
	UA_Boolean                allowAnonymous;
	size_t                    usernamePasswordLoginSize;
	UA_UsernamePasswordLogin *usernamePasswordLogin;
} AccessControlContext;
#define ANONYMOUS_POLICY "open62541-anonymous-policy"
#define USERNAME_POLICY  "open62541-username-policy"
const UA_String anonymous_policy = UA_STRING_STATIC(ANONYMOUS_POLICY);
const UA_String username_policy  = UA_STRING_STATIC(USERNAME_POLICY);

UA_StatusCode QUaServer::uaConstructor(UA_Server       * server, 
	                                   const UA_NodeId * sessionId, 
	                                   void            * sessionContext, 
	                                   const UA_NodeId * typeNodeId, 
	                                   void            * typeNodeContext, 
	                                   const UA_NodeId * nodeId, 
	                                   void            ** nodeContext)
{
	Q_UNUSED(server);
	Q_UNUSED(sessionId); 
	Q_UNUSED(sessionContext); 
	// get server from context object
	auto srv = dynamic_cast<QUaServer*>(static_cast<QObject*>(typeNodeContext));
	Q_CHECK_PTR(srv);
	if (!srv)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	Q_ASSERT(srv->m_hashConstructors.contains(*typeNodeId));
	// get method from type constructors map and call it
	auto st = srv->m_hashConstructors[*typeNodeId](nodeId, nodeContext);
	// emit new instance signal if appropriate
	if (srv->m_hashSignalers.contains(*typeNodeId))
	{
		auto signaler = srv->m_hashSignalers.value(*typeNodeId);
		auto newInstance = dynamic_cast<QUaNode*>(static_cast<QObject*>(*nodeContext));
		Q_CHECK_PTR(newInstance);
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
	Q_UNUSED(sessionId);
	void * context;
	auto st = UA_Server_getNodeContext(server, *nodeId, &context);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// try to convert to node
	auto node = dynamic_cast<QUaNode*>(static_cast<QObject*>(context));
	// early exit if not convertible (this call was triggered by ~QUaNode)
	if (!node)
	{
		return;
	}
	// handle events if enabled
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// check if event (not in tree)
	auto evt = dynamic_cast<QUaBaseEvent*>(node);
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
	auto parentNode = dynamic_cast<QUaNode*>(static_cast<QObject*>(parentContext));
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

UA_StatusCode QUaServer::uaConstructor(QUaServer         * server,
	                                   const UA_NodeId   * nodeId, 
	                                   void             ** nodeContext,
	                                   const QMetaObject & metaObject)
{
	// get parent node id
	UA_NodeId parentNodeId       = QUaNode::getParentNodeId(*nodeId, server->m_server);
	// handle events
#ifndef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	Q_ASSERT(!UA_NodeId_isNull(&parentNodeId));
#else
	Q_ASSERT(!UA_NodeId_isNull(&parentNodeId) || metaObject.inherits(&QUaBaseEvent::staticMetaObject));
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// check if constructor from explicit instance creation or type registration
	// this is done by checking that parent is different from object ot variable instance
	UA_NodeClass outNodeClass;
	UA_NodeId lastParentNodeId;
	QUaNode * parentContext = nullptr;
	while (true)
	{
		// handle events
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		// check if event
		if (metaObject.inherits(&QUaBaseEvent::staticMetaObject))
		{
			break;
		}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
		// ignore if new instance is due to new type registration
		UA_Server_readNodeClass(server->m_server, parentNodeId, &outNodeClass);
		Q_ASSERT_X(outNodeClass != UA_NODECLASS_UNSPECIFIED, "QUaServer::uaConstructor", "Something went wrong while getting parent info.");
		if(outNodeClass != UA_NODECLASS_OBJECT && outNodeClass != UA_NODECLASS_VARIABLE)
		{
			UA_NodeId_clear(&parentNodeId);
			return UA_STATUSCODE_GOOD;
		}
		lastParentNodeId = parentNodeId;
		parentContext = QUaNode::getNodeContext(lastParentNodeId, server);
		// check if parent node is bound
		if (parentContext)
		{
			break;
		}
		auto tmpParentNodeId = QUaNode::getParentNodeId(parentNodeId, server->m_server);
		UA_NodeId_clear(&parentNodeId); // clear old
		parentNodeId = tmpParentNodeId;
		// exit if null
		if (UA_NodeId_isNull(&parentNodeId))
		{
			break;
		}
	}
	// create new instance (and bind it to UA, in base types happens in constructor, in derived class is done by QOpcUaServerNodeFactory)
	Q_ASSERT_X(metaObject.constructorCount() > 0, "QUaServer::uaConstructor", "Failed instantiation. No matching Q_INVOKABLE constructor with signature CONSTRUCTOR(QUaServer *server, const UA_NodeId &nodeId) found.");
	// NOTE : to simplify user API, we minimize QUaNode arguments to just a QUaServer reference
	//        we temporarily store in the QUaServer reference the UA_NodeId and QMetaObject values needed to
	//        instantiate the new node.
	server->m_newNodeNodeId     = nodeId;
	server->m_newNodeMetaObject = &metaObject;
	// instantiate new C++ node, m_newNodeNodeId and m_newNodeMetaObject only meant to be used during this call
	auto * pQObject    = metaObject.newInstance(Q_ARG(QUaServer*, server));
	Q_ASSERT_X(pQObject, "QUaServer::uaConstructor", "Failed instantiation. No matching Q_INVOKABLE constructor with signature CONSTRUCTOR(QUaServer *server, const UA_NodeId &nodeId) found.");
	auto * newInstance = dynamic_cast<QUaNode*>(static_cast<QObject*>(pQObject));
	Q_CHECK_PTR(newInstance);
	if (!newInstance)
	{
		UA_NodeId_clear(&parentNodeId);
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	// need to bind again using the official (void ** nodeContext) of the UA constructor
	// because we set context on C++ instantiation, but later the UA library overwrites it 
	// after calling the UA constructor
	*nodeContext = (void*)newInstance;
	newInstance->m_nodeId = *nodeId;
	// need to set parent if direct parent is already bound bacause its constructor has already been called
	UA_NodeId_clear(&parentNodeId);
	parentNodeId = QUaNode::getParentNodeId(*nodeId, server->m_server);
	if (UA_NodeId_equal(&parentNodeId, &lastParentNodeId) && parentContext)
	{
		newInstance->setParent(parentContext);
		newInstance->setObjectName(QUaNode::getBrowseName(*nodeId, server));
		// emit child added to parent
		emit parentContext->childAdded(newInstance);
	}
	// success
	UA_NodeId_clear(&parentNodeId);
	return UA_STATUSCODE_GOOD;
}

// [STATIC]
UA_StatusCode QUaServer::methodCallback(UA_Server        * server,
	                                    const UA_NodeId  * sessionId, 
	                                    void             * sessionContext, 
	                                    const UA_NodeId  * methodId, 
	                                    void             * methodContext, 
	                                    const UA_NodeId  * objectId, 
	                                    void             * objectContext, 
	                                    size_t             inputSize, 
	                                    const UA_Variant * input, 
	                                    size_t             outputSize, 
	                                    UA_Variant       * output)
{
	Q_UNUSED(server        );
	Q_UNUSED(sessionId     );
	Q_UNUSED(sessionContext);
	Q_UNUSED(objectId      );
	Q_UNUSED(inputSize     );
	Q_UNUSED(outputSize    );
	// get node from context object
	auto srv = dynamic_cast<QUaServer*>(static_cast<QObject*>(methodContext));
	Q_CHECK_PTR(srv);
	if (!srv)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	Q_ASSERT(srv->m_hashMethods.contains(*methodId));
	// get method from node callbacks map and call it
	return srv->m_hashMethods[*methodId](objectContext, input, output);
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
	auto qServer = dynamic_cast<QUaServer*>(static_cast<QObject*>(context));
	Q_CHECK_PTR(qServer);
	return qServer;
}

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
	arrayDimensions[0] = 0;
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
	QUaServer *qServer = QUaServer::getServerNodeContext(server);
	/* The empty token is interpreted as anonymous */
	if (userIdentityToken->encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY) {
		if (!context->allowAnonymous)
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		// NOTE : custom code : add session to hash
		Q_ASSERT(!qServer->m_hashSessions.contains(*sessionId));
		qServer->m_hashSessions[*sessionId] = "";

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
		//Q_ASSERT(!qServer->m_hashSessions.contains(*sessionId));
		qServer->m_hashSessions[*sessionId] = "";

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

		/* TODO: Support encrypted username/password over unencrypted SecureChannels */
		if (userToken->encryptionAlgorithm.length > 0)
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		/* Empty username and password */
		if (userToken->userName.length == 0 && userToken->password.length == 0)
			return UA_STATUSCODE_BADIDENTITYTOKENINVALID;

		
		/* Try to match username/pw */

		// NOTE : custom code : check user and password
		const QString userName = QString::fromUtf8((char*)userToken->userName.data, (int)userToken->userName.length);
		const QString password = QString::fromUtf8((char*)userToken->password.data, (int)userToken->password.length);	
		// Call validation callback
		UA_Boolean match = qServer->m_validationCallback(userName, password);
		if (!match)
		{
			return UA_STATUSCODE_BADUSERACCESSDENIED;			
		}
		else if (match && !qServer->m_hashUsers.contains(userName))
		{
			// Server must be aware of user name otherwise it will kick user out of session
			// in user access callbacks
			// NOTE : do not keep password in memory for security
			qServer->m_hashUsers.insert(userName, "");
		}

		// NOTE : actually is possible for a current session to change its user while maintaining nodeId
		//Q_ASSERT(!qServer->m_hashSessions.contains(*sessionId));

		// NOTE : custom code : add session to hash
		qServer->m_hashSessions[*sessionId] = userName;

		/* No userdata atm */
		*sessionContext = NULL;

		return (UA_StatusCode)UA_STATUSCODE_GOOD;
	}

	/* Unsupported token type */
	return UA_STATUSCODE_BADIDENTITYTOKENINVALID;
}

void QUaServer::closeSession(UA_Server        * server, 
	                         UA_AccessControl * ac, 
	                         const UA_NodeId  * sessionId, 
	                         void             * sessionContext)
{
	Q_UNUSED(sessionContext);
	Q_UNUSED(ac);
	// get server
	QUaServer *qServer = QUaServer::getServerNodeContext(server);
	// remove session form hash
	qServer->m_hashSessions.remove(*sessionId);
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
	QUaServer *qServer = QUaServer::getServerNodeContext(server);
	Q_ASSERT(qServer->m_hashSessions.contains(*sessionId));
	// get user
	QString strUserName = qServer->m_hashSessions.value(*sessionId);
	// check if user still exists
	if (!strUserName.isEmpty() && !qServer->userExists(strUserName))
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
	QUaServer *qServer = QUaServer::getServerNodeContext(server);
	Q_ASSERT(qServer->m_hashSessions.contains(*sessionId));
	// get user
	QString strUserName = qServer->m_hashSessions.value(*sessionId);
	// check if user still exists
	if (!strUserName.isEmpty() && !qServer->userExists(strUserName))
	{
		// TODO : wait until officially supported
		// https://github.com/open62541/open62541/issues/2617
		//auto st = UA_Server_closeSession(server, sessionId);
		//Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return (UA_UInt32)0;
	}
	// if node from user tree then call user implementation
	QUaNode * node = QUaNode::getNodeContext(*nodeId, server);
	QUaBaseVariable * variable = dynamic_cast<QUaBaseVariable *>(node);
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
	QUaServer *qServer = QUaServer::getServerNodeContext(server);
	Q_ASSERT(qServer->m_hashSessions.contains(*sessionId));
	// get user
	QString strUserName = qServer->m_hashSessions.value(*sessionId);
	// check if user still exists
	if (!strUserName.isEmpty() && !qServer->userExists(strUserName))
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
	QUaServer *qServer = QUaServer::getServerNodeContext(server);
	Q_ASSERT(qServer->m_hashSessions.contains(*sessionId));
	// get user
	QString strUserName = qServer->m_hashSessions.value(*sessionId);
	// check if user still exists
	if (!strUserName.isEmpty() && !qServer->userExists(strUserName))
	{
		// TODO : wait until officially supported
		// https://github.com/open62541/open62541/issues/2617
		//auto st = UA_Server_closeSession(server, sessionId);
		//Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return false;
	}
	// if node from user tree then call user implementation
	QUaNode * node = QUaNode::getNodeContext(*objectId, server);
	QUaBaseObject * object = dynamic_cast<QUaBaseObject *>(node);
	if (object)
	{
		// NOTE : could not diff by method name because name multiples are possible
		return object->userExecutableInternal(strUserName);
	}
	// else default
	return true;
}

#ifndef UA_ENABLE_ENCRYPTION
// NOTE : cannot load cert later with UA_Server_updateCertificate because
//        it requires oldCertificate != NULL
QUaServer::QUaServer(const quint16    &intPort        /* = 4840*/, 
	                 const QByteArray &byteCertificate/* = QByteArray()*/, 
	                 QObject          *parent         /* = 0*/) 
	: QObject(parent)
{
	m_port = intPort;
	// convert cert if valid
	UA_ByteString cert;
	UA_ByteString * ptrCert = QUaServer::parseCertificate(byteCertificate, cert, m_byteCertificate);
	// create config with port and certificate
	this->m_server = UA_Server_new();
	UA_ServerConfig_setMinimal(UA_Server_getConfig(this->m_server), intPort, ptrCert);
	// setup server
	this->setupServer();
}
#else
QUaServer::QUaServer(const quint16    & intPort        /* = 4840*/,
	                 const QByteArray & byteCertificate/* = QByteArray()*/,
	                 const QByteArray & bytePrivateKey /* = QByteArray()*/,
	                 QObject          * parent         /* = 0*/)
	: QObject(parent)
{
	m_port = intPort;
	this->m_server = UA_Server_new();
	// convert cert if valid (should contain public key)
	UA_ByteString cert;
	UA_ByteString * ptrCert = QUaServer::parseCertificate(byteCertificate, cert, m_byteCertificate);
	// convert private key if valid
	UA_ByteString priv;
	UA_ByteString * ptrPriv = QUaServer::parseCertificate(bytePrivateKey, priv, m_bytePrivateKey);
	// check if valid private key
	if (ptrPriv)
	{
		// create config with port, certificate and private key for encryption
		UA_ServerConfig_setDefaultWithSecurityPolicies(
			UA_Server_getConfig(this->m_server), 
			intPort, 
			ptrCert, 
			ptrPriv, 
			nullptr, 
			0, 
			nullptr, 
			0,
			nullptr,
			0
		);
	}
	else
	{
		// create config with port and certificate only (no encryption)
		UA_ServerConfig_setMinimal(UA_Server_getConfig(this->m_server), intPort, ptrCert);
	}	
	// setup server
	this->setupServer();
}
#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
void QUaServer::addChange(const QUaChangeStructureDataType& change)
{
	Q_CHECK_PTR(m_changeEvent);
	m_listChanges.append(change);
	m_triggerChanges();
}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

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
	// Qt Stuff
	if (QMetaType::type("QUaReference") == QMetaType::UnknownType)
	{
		qRegisterMetaType<QUaReference>("QUaReference");
	}
	if (QMetaType::type("QUaEnumEntry") == QMetaType::UnknownType)
	{
		qRegisterMetaType<QUaEnumEntry>("QUaEnumEntry");
	}
	// Server stuff
	UA_StatusCode st;
	m_running = false;
	// Set default validation callback
	m_validationCallback = [this](const QString &strUserName, const QString &strPassword) {
		if (!m_hashUsers.contains(strUserName))
		{
			return false;
		}
		return m_hashUsers[strUserName].compare(strPassword, Qt::CaseSensitive) == 0;
	};
	// Create "Objects" folder using special constructor
	// Part 5 - 8.2.4 : Objects
	auto objectsNodeId        = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	this->m_newNodeNodeId     = &objectsNodeId;
	this->m_newNodeMetaObject = &QUaFolderObject::staticMetaObject;
	m_pobjectsFolder = new QUaFolderObject(this);
	m_pobjectsFolder->setParent(this);
	m_pobjectsFolder->setObjectName("Objects");
	// register base types (for all types)
	m_mapTypes.insert(QString(QUaBaseVariable    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE    ));
	m_mapTypes.insert(QString(QUaBaseDataVariable::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE));
	m_mapTypes.insert(QString(QUaProperty        ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE        ));
	m_mapTypes.insert(QString(QUaBaseObject      ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE      ));
	m_mapTypes.insert(QString(QUaFolderObject    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE          ));
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	m_mapTypes.insert(QString(QUaBaseEvent              ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE              ));
	m_mapTypes.insert(QString(QUaGeneralModelChangeEvent::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_GENERALMODELCHANGEEVENTTYPE));
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// set context for server
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)              , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// set context for base types (for instantiable types)
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_GENERALMODELCHANGEEVENTTYPE), (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	Q_UNUSED(st);
	// register constructors (for instantiable types)
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), QUaBaseDataVariable::staticMetaObject);
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , QUaProperty        ::staticMetaObject);
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , QUaBaseObject      ::staticMetaObject);
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , QUaFolderObject    ::staticMetaObject);
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_GENERALMODELCHANGEEVENTTYPE), QUaGeneralModelChangeEvent::staticMetaObject);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// setup access control
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
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

	// get server app name
	m_byteApplicationName = QByteArray::fromRawData(
		(char*)config->applicationDescription.applicationName.text.data,
		(int)config->applicationDescription.applicationName.text.length
	);
	// get server app uri
	m_byteApplicationUri = QByteArray::fromRawData(
		(char*)config->applicationDescription.applicationUri.data,
		(int)config->applicationDescription.applicationUri.length
	);

	// get server product name
	m_byteProductName = QByteArray::fromRawData(
		(char*)config->buildInfo.productName.data,
		(int)config->buildInfo.productName.length
	);
	// get server product uri
	m_byteProductUri = QByteArray::fromRawData(
		(char*)config->buildInfo.productUri.data,
		(int)config->buildInfo.productUri.length
	);
	// get server manufacturer name
	m_byteManufacturerName = QByteArray::fromRawData(
		(char*)config->buildInfo.manufacturerName.data,
		(int)config->buildInfo.manufacturerName.length
	);
	// get server software version
	m_byteSoftwareVersion = QByteArray::fromRawData(
		(char*)config->buildInfo.softwareVersion.data,
		(int)config->buildInfo.softwareVersion.length
	);
	// get server software version
	m_byteBuildNumber = QByteArray::fromRawData(
		(char*)config->buildInfo.buildNumber.data,
		(int)config->buildInfo.buildNumber.length
	);

	// instantiate change event
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	m_changeEvent = this->createEvent<QUaGeneralModelChangeEvent>();
	Q_CHECK_PTR(m_changeEvent);
	m_changeEvent->setSourceName(this->applicationName());
	m_changeEvent->setMessage("Node added or removed.");
	m_changeEvent->setSeverity(1);
	// create debounced trigerring function
	m_triggerChanges = QFunctionUtils::Debounce(
	[this]() {
		// trigger
		m_changeEvent->setChanges(m_listChanges);
		m_changeEvent->setTime(QDateTime::currentDateTimeUtc());
		m_changeEvent->trigger();
		// clean list of changes buffer
		m_listChanges.clear();
	}, QUA_DEBOUNCE_PERIOD_MS);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
}

QUaServer::~QUaServer()
{
	// [FIX] : QObject children destructors were called after this one
	//         and the ~QUaNode destructor makes use of m_server
	//         so we better destroy the children manually before deleting m_server
	while (this->children().count() > 0)
	{
		delete this->children().at(0);
	}
	// Cleanup library objects
	UA_Server_delete(this->m_server);
}

quint16 QUaServer::port() const
{
	return m_port;
}

QString QUaServer::applicationName() const
{
	return QString(m_byteApplicationName);
}

/*
// NOTE : not use about updating config->applicationDescription. 
//        In UA_ServerConfig_new_customBuffer we have

conf->endpointsSize = 1;

// which seems to point that there will only be one endpoint
// so maybe we can do the same as in createEndpoint and just copy 
// the applicationDescription from config to endpoint
*/

void QUaServer::setApplicationName(const QString & strApplicationName)
{
	// update config
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	m_byteApplicationName = strApplicationName.toUtf8();
	config->applicationDescription.applicationName = UA_LOCALIZEDTEXT((char*)"", m_byteApplicationName.data());
	// update endpoints
	UA_ApplicationDescription_copy(&config->applicationDescription, &config->endpoints[0].server);
	// update application name on change event
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	Q_CHECK_PTR(m_changeEvent);
	m_changeEvent->setSourceName(strApplicationName);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
}

QString QUaServer::applicationUri() const
{
	return QString(m_byteApplicationUri);
}

void QUaServer::setApplicationUri(const QString & strApplicationUri)
{
	// update config
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	m_byteApplicationUri = strApplicationUri.toUtf8();
	config->applicationDescription.applicationUri = UA_STRING(m_byteApplicationUri.data());
	// update endpoints
	UA_ApplicationDescription_copy(&config->applicationDescription, &config->endpoints[0].server);
}

QString QUaServer::productName() const
{
	return QString(m_byteProductName);
}

void QUaServer::setProductName(const QString & strProductName)
{
	// update config
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	m_byteProductName = strProductName.toUtf8();
	config->buildInfo.productName = UA_STRING(m_byteProductName.data());
}

QString QUaServer::productUri() const
{
	return QString(m_byteProductUri);
}

void QUaServer::setProductUri(const QString & strProductUri)
{
	// update config
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	m_byteProductUri = strProductUri.toUtf8();
	config->buildInfo.productUri = UA_STRING(m_byteProductUri.data());
	// NOTE : update application description productUri as well
	config->applicationDescription.productUri = UA_STRING(m_byteProductUri.data());
	// update endpoints
	UA_ApplicationDescription_copy(&config->applicationDescription, &config->endpoints[0].server);
}

QString QUaServer::manufacturerName() const
{
	return QString(m_byteManufacturerName);
}

void QUaServer::setManufacturerName(const QString & strManufacturerName)
{
	// update config
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	m_byteManufacturerName = strManufacturerName.toUtf8();
	config->buildInfo.manufacturerName = UA_STRING(m_byteManufacturerName.data());
}

QString QUaServer::softwareVersion() const
{
	return QString(m_byteSoftwareVersion);
}

void QUaServer::setSoftwareVersion(const QString & strSoftwareVersion)
{
	// update config
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	m_byteSoftwareVersion = strSoftwareVersion.toUtf8();
	config->buildInfo.softwareVersion = UA_STRING(m_byteSoftwareVersion.data());
}

QString QUaServer::buildNumber() const
{
	return QString(m_byteBuildNumber);
}

void QUaServer::setBuildNumber(const QString & strBuildNumber)
{
	// update config
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	m_byteBuildNumber = strBuildNumber.toUtf8();
	config->buildInfo.buildNumber = UA_STRING(m_byteBuildNumber.data());
}

void QUaServer::start()
{
	// NOTE : we must define port and other server params upon instantiation, 
	//        because rest of API assumes m_server is valid
	if (m_running)
	{
		return;
	}
	auto st = UA_Server_run_startup(m_server);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st)
	m_running = true;
	QObject::connect(&m_iterWaitTimer, &QTimer::timeout, this,
	[this]() {
		// do not iterate if asked to stop
		if (!m_running) { return; }
		// iterate and restart
		UA_Server_run_iterate(m_server, false);
	});
	// start iterations
	// NOTE : using a 1ms delay seems to have the best trade-off (so far) between
	//        not blocking Qt's event loop, OPC server's resposiveness, and not overloading the CPU.
	//        I made some tests changing the system's local time on winows and QTimer does not
	//        seem affected by it at least in Qt 5.12, don't know if using QTimer for this
	//        will come back bite me in the ass some time later.
	m_iterWaitTimer.start(1);
}

// NOTE : copied from open62541 to help resolve QUaServer::stop/start issue
static UA_StatusCode addDefaultNetworkLayers(
	UA_ServerConfig *conf, 
	UA_UInt16 portNumber,
	UA_UInt32 sendBufferSize, 
	UA_UInt32 recvBufferSize) 
{
	/* Add a network layer */
	conf->networkLayers = (UA_ServerNetworkLayer *)
		UA_malloc(sizeof(UA_ServerNetworkLayer));
	if (!conf->networkLayers)
		return UA_STATUSCODE_BADOUTOFMEMORY;

	UA_ConnectionConfig config = UA_ConnectionConfig_default;
	if (sendBufferSize > 0)
		config.sendBufferSize = sendBufferSize;
	if (recvBufferSize > 0)
		config.recvBufferSize = recvBufferSize;

	conf->networkLayers[0] =
		UA_ServerNetworkLayerTCP(config, portNumber, &conf->logger);
	if (!conf->networkLayers[0].handle)
		return UA_STATUSCODE_BADOUTOFMEMORY;
	conf->networkLayersSize = 1;

	return UA_STATUSCODE_GOOD;
}

void QUaServer::stop()
{
	m_running = false;
	m_iterWaitTimer.stop();
	m_iterWaitTimer.disconnect();
	UA_Server_run_shutdown(m_server);
	// NOTE : there is a bug in open62541, if server is shutdown while a client still connected
	//        then after starting server again, clients cannot connect to it anymore.
	//        the issue seems to be in *ServerNetworkLayerTCP_listen*, where the server gets
	//        stucked in the "Socket select failed with %s" error.
	//        the code below helps, but sometimes crashes when calling UA_Server_delete (i.e. ~QUaServer)
	// cleanup network layers
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	for (size_t i = 0; i < config->networkLayersSize; ++i) {
		UA_ServerNetworkLayer *nl = &config->networkLayers[i];
		nl->deleteMembers(nl);
	}
	UA_free(config->networkLayers);
	// recreate network layers (as in UA_ServerConfig_setMinimalCustomBuffer)
	auto retval = addDefaultNetworkLayers(config, m_port, 0, 0);
	Q_ASSERT(retval == UA_STATUSCODE_GOOD);
}

bool QUaServer::isRunning() const
{
	return m_running;
}

quint16 QUaServer::maxSecureChannels() const
{
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	return config->maxSecureChannels;
}

void QUaServer::setMaxSecureChannels(const quint16 & maxSecureChannels)
{
	UA_ServerConfig * config  = UA_Server_getConfig(m_server);
	config->maxSecureChannels = maxSecureChannels;
}

quint16 QUaServer::maxSessions() const
{
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	return config->maxSessions;
}

void QUaServer::setMaxSessions(const quint16 & maxSessions)
{
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	config->maxSessions = maxSessions;
}

void QUaServer::registerType(const QMetaObject &metaObject, const QString &strNodeId/* = ""*/)
{
	// check if OPC UA relevant
	if (!metaObject.inherits(&QUaNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::registerType", "Unsupported base class");
		return;
	}
	// check if already registered
	QString   strClassName  = QString(metaObject.className());
	UA_NodeId newTypeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (!UA_NodeId_isNull(&newTypeNodeId))
	{
		// add to map of not here yet
		if (!m_mapTypes.contains(strClassName))
		{
			m_mapTypes[strClassName] = newTypeNodeId;
		}
		return;
	}
	// create new type browse name
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name = QUaTypesConverter::uaStringFromQString(strClassName);
	// check if base class is registered
	QString strBaseClassName = QString(metaObject.superClass()->className());
	if (!m_mapTypes.contains(strBaseClassName))
	{
		// recursive
		this->registerType(*metaObject.superClass());
	}
	Q_ASSERT_X(m_mapTypes.contains(strBaseClassName), "QUaServer::registerType", "Base object type not registered.");
	// check if requested node id defined
	QString strReqNodeId = strNodeId.trimmed();
	UA_NodeId reqNodeId  = strReqNodeId.isEmpty() ? UA_NODEID_NULL : QUaTypesConverter::nodeIdFromQString(strReqNodeId);
	// check if requested node id exists
	UA_NodeId outNodeId;
	auto st = UA_Server_readNodeId(m_server, reqNodeId, &outNodeId);
	Q_ASSERT_X(st == UA_STATUSCODE_BADNODEIDUNKNOWN, "QUaServer::registerType", "Requested NodeId already exists");
	if (st != UA_STATUSCODE_BADNODEIDUNKNOWN)
	{
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return;
	}
	// check if variable or object
	if (metaObject.inherits(&QUaBaseDataVariable::staticMetaObject))
	{
		// create variable type attributes
		UA_VariableTypeAttributes vtAttr = UA_VariableTypeAttributes_default;
		// set node attributes		  
		QByteArray byteDisplayName       = strClassName.toUtf8();
		vtAttr.displayName               = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
		QByteArray byteDescription       = QString("").toUtf8();
		vtAttr.description               = UA_LOCALIZEDTEXT((char*)"", byteDescription.data());
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
		QByteArray byteDisplayName     = strClassName.toUtf8();
		otAttr.displayName             = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
		QByteArray byteDescription     = QString("").toUtf8();
		otAttr.description             = UA_LOCALIZEDTEXT((char*)"", byteDescription.data());
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
	// add to registered types map
	m_mapTypes.insert(strClassName, newTypeNodeId);
	// register constructor/destructor
	this->registerTypeLifeCycle(&newTypeNodeId, metaObject);
	// register meta-enums
	this->registerMetaEnums(metaObject);
	// register meta-properties
	this->addMetaProperties(metaObject);
	// register meta-methods (only if object class, or NOT variable class)
	if (!metaObject.inherits(&QUaBaseDataVariable::staticMetaObject))
	{
		this->addMetaMethods(metaObject);
	}
}

QList<QUaNode*> QUaServer::typeInstances(const QMetaObject & metaObject)
{
	QList<QUaNode*> retList;
	// check if OPC UA relevant
	if (!metaObject.inherits(&QUaNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::typeInstances", "Unsupported base class. It must derive from QUaNode");
		return retList;
	}
	// try to get typeNodeId, if null, then register it
	QString   strClassName = QString(metaObject.className());
	UA_NodeId typeNodeId   = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (UA_NodeId_isNull(&typeNodeId))
	{
		this->registerType(metaObject);
		typeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	}
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));

	// make ua browse
	auto refTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
	QList<UA_NodeId> retRefSet;
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	UA_NodeId_copy(&typeNodeId, &bDesc->nodeId); 
	bDesc->browseDirection = UA_BROWSEDIRECTION_INVERSE; //TypeDefinitionOf
	bDesc->includeSubtypes = true;
	bDesc->referenceTypeId = refTypeId;
	bDesc->resultMask      = UA_BROWSERESULTMASK_REFERENCETYPEID;
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
		UA_BrowseResult_deleteMembers(&bRes);
		bRes = UA_Server_browseNext(m_server, true, &bRes.continuationPoint);
	}
	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);
	// get QUaNode references
	for (int i = 0; i < retRefSet.count(); i++)
	{
		retList << QUaNode::getNodeContext(retRefSet[i], m_server);
		UA_NodeId_clear(&retRefSet[i]);
	}
	return retList;
}

void QUaServer::registerEnum(const QMetaEnum & metaEnum, const QString &strNodeId/* = ""*/)
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
	this->registerEnum(strBrowseName, mapEnum, strNodeId);
}

void QUaServer::registerTypeLifeCycle(const UA_NodeId &typeNodeId, const QMetaObject &metaObject)
{
    this->registerTypeLifeCycle(&typeNodeId, metaObject);
}

void QUaServer::registerTypeLifeCycle(const UA_NodeId * typeNodeId, const QMetaObject & metaObject)
{
	Q_CHECK_PTR(typeNodeId);
	Q_ASSERT(!UA_NodeId_isNull(typeNodeId));
	if (UA_NodeId_isNull(typeNodeId))
	{
		return;
	}
	// add constructor
	Q_ASSERT_X(!m_hashConstructors.contains(*typeNodeId), "QUaServer::registerType", "Constructor for type already exists.");
	// NOTE : we need constructors to be lambdas in order to cache metaobject in capture
	//        because type context is already the server instance where type was registered
	//        so we can differentiate the server instance in the static ::uaConstructor callback
	//        TLDR; to support multiple server instances in an application
	m_hashConstructors[*typeNodeId] = [metaObject, this](const UA_NodeId *instanceNodeId, void ** nodeContext) {
		// call static method
		return QUaServer::uaConstructor(this, instanceNodeId, nodeContext, metaObject);
	};

	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &QUaServer::uaConstructor;
	lifecycle.destructor  = &QUaServer::uaDestructor;
	
	auto st = UA_Server_setNodeTypeLifecycle(m_server, *typeNodeId, lifecycle);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st)
}

void QUaServer::registerMetaEnums(const QMetaObject & parentMetaObject)
{
	int enumCount = parentMetaObject.enumeratorCount();
	for (int i = parentMetaObject.enumeratorOffset(); i < enumCount; i++)
	{
		QMetaEnum metaEnum = parentMetaObject.enumerator(i);
		this->registerEnum(metaEnum);
	}
}

void QUaServer::addMetaProperties(const QMetaObject & parentMetaObject)
{
	QString   strParentClassName = QString(parentMetaObject.className());
	UA_NodeId parentTypeNodeId   = m_mapTypes.value(strParentClassName, UA_NODEID_NULL);
	Q_ASSERT(!UA_NodeId_isNull(&parentTypeNodeId));
	// loop meta properties and find out which ones inherit from
	int propCount = parentMetaObject.propertyCount();
	for (int i = parentMetaObject.propertyOffset(); i < propCount; i++)
	{
		QMetaProperty metaProperty = parentMetaObject.property(i);
		// check if is meta enum
		bool      isVariable     = false;
		bool      isEnum         = false;
		UA_NodeId enumTypeNodeId = UA_NODEID_NULL;
		if (metaProperty.isEnumType())
		{
			QMetaEnum metaEnum = metaProperty.enumerator();
			// compose enum name
			#if QT_VERSION >= 0x051200
				QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
			#else
				QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.name());
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
		QString    strPropName  = QString(metaProperty.name());
		QByteArray bytePropName = strPropName.toUtf8();
		// get type node id
		UA_NodeId propTypeNodeId;
		if (!isEnum)
		{
			// check if available in meta-system
			if (!QMetaType::metaObjectForType(metaProperty.userType()) && !isEnum)
			{
				continue;
			}
			// check if OPC UA relevant type
			const QMetaObject propMetaObject = *QMetaType::metaObjectForType(metaProperty.userType());
			if (!propMetaObject.inherits(&QUaNode::staticMetaObject))
			{
				continue;
			}
			// check if prop inherits from parent
			Q_ASSERT_X(!propMetaObject.inherits(&parentMetaObject), "QUaServer::addMetaProperties", "Qt MetaProperty type cannot inherit from Class.");
			if (propMetaObject.inherits(&parentMetaObject) && !isEnum)
			{
				continue;
			}
			// check if prop type registered, register of not
			QString strPropClassName = QString(propMetaObject.className());
			propTypeNodeId = m_mapTypes.value(strPropClassName, UA_NODEID_NULL);
			if (UA_NodeId_isNull(&propTypeNodeId))
			{
				this->registerType(propMetaObject);
				propTypeNodeId = m_mapTypes.value(strPropClassName, UA_NODEID_NULL);
			}
			// set is variable
			isVariable = propMetaObject.inherits(&QUaBaseVariable::staticMetaObject);
			// check if ua property, then set correct reference
			if (propMetaObject.inherits(&QUaProperty::staticMetaObject))
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
		browseName.namespaceIndex = 1;
		browseName.name           = QUaTypesConverter::uaStringFromQString(strPropName);
		// display name
		UA_LocalizedText displayName = UA_LOCALIZEDTEXT((char*)"", bytePropName.data());
		// check if variable or object
		// NOTE : a type is considered to inherit itself sometimes does not work (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
		UA_NodeId tempNodeId;
		if (isVariable || isEnum)
		{
			
			UA_VariableAttributes vAttr = UA_VariableAttributes_default;
			vAttr.displayName = displayName;
			// if enum, set data type
			if (isEnum)
			{
				Q_ASSERT(!UA_NodeId_isNull(&enumTypeNodeId));
				vAttr.dataType = enumTypeNodeId;
			}
			// add variable
			auto st = UA_Server_addVariableNode(m_server,
												UA_NODEID_NULL,   // requested nodeId
												parentTypeNodeId, // parent
												referenceTypeId,  // parent relation with child
												browseName,		  
												propTypeNodeId,   
												vAttr, 			  
												nullptr,          // context
												&tempNodeId);     // output nodeId to make mandatory
			Q_ASSERT(st == UA_STATUSCODE_GOOD);
			Q_UNUSED(st);
		}
		else
		{
			// NOTE : not working ! Q_ASSERT(propMetaObject.inherits(&QUaBaseObject::staticMetaObject));
			UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
			oAttr.displayName = displayName;
			// add object
			auto st = UA_Server_addObjectNode(m_server,
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

void QUaServer::addMetaMethods(const QMetaObject & parentMetaObject)
{
	QString   strParentClassName = QString(parentMetaObject.className());
	UA_NodeId parentTypeNodeId   = m_mapTypes.value(strParentClassName, UA_NODEID_NULL);
	Q_ASSERT(!UA_NodeId_isNull(&parentTypeNodeId));
	// loop meta methods and find out which ones inherit from
	int methCount = parentMetaObject.methodCount();
	for (int i = parentMetaObject.methodOffset(); i < methCount; i++)
	{
		QMetaMethod metamethod = parentMetaObject.method(i);
		// validate id method (not signal, slot or constructor)
		auto methodType = metamethod.methodType();
		if (methodType != QMetaMethod::Method)
		{
			continue;
		}
		// validate return type
		auto returnType  = (QMetaType::Type)metamethod.returnType();
		bool isSupported = QUaTypesConverter::isSupportedQType(returnType);
		bool isEnumType  = this->m_hashEnums.contains(metamethod.typeName());
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
		UA_Argument   outputArgumentInstance;
		UA_Argument * outputArgument = nullptr;
		if (returnType != QMetaType::Void)
		{
			UA_Argument_init(&outputArgumentInstance);
			outputArgumentInstance.description = UA_LOCALIZEDTEXT((char *)"",
														  (char *)"Result Value");
			outputArgumentInstance.name      = QUaTypesConverter::uaStringFromQString((char *)"Result");
			outputArgumentInstance.dataType  = isEnumType ? 
				this->m_hashEnums.value(QString(metamethod.typeName())) :
				QUaTypesConverter::uaTypeNodeIdFromQType(returnType);
			outputArgumentInstance.valueRank = isArrayType ?
				UA_VALUERANK_ONE_DIMENSION :
				UA_VALUERANK_SCALAR;
			outputArgument = &outputArgumentInstance;
		}
		// validate argument types and create them
		Q_ASSERT_X(metamethod.parameterCount() <= 10, 
			"QUaServer::addMetaMethods", 
			"No more than 10 arguments supported in MetaMethod.");
		if (metamethod.parameterCount() > 10)
		{
			continue;
		}
		QVector<UA_Argument> vectArgs;
		auto listArgNames = metamethod.parameterNames();
		Q_ASSERT(listArgNames.count() == metamethod.parameterCount());
		auto listTypeNames = metamethod.parameterTypes();
		for (int k = 0; k < metamethod.parameterCount(); k++)
		{
			auto argType = (QMetaType::Type)metamethod.parameterType(k);
			isSupported  = QUaTypesConverter::isSupportedQType(argType);
			isEnumType   = this->m_hashEnums.contains(listTypeNames[k]);
			isArrayType  = QUaTypesConverter::isQTypeArray(argType);
			isValidType  = isSupported || isEnumType || isArrayType;
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
			inputArgument.description = UA_LOCALIZEDTEXT((char *)"", (char *)"Method Argument");
			inputArgument.name        = QUaTypesConverter::uaStringFromQString(listArgNames[k]);
			inputArgument.dataType    = uaType;
			inputArgument.valueRank   = UA_VALUERANK_SCALAR;
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
		auto strMethName = metamethod.name();
		// add method node
		UA_MethodAttributes methAttr = UA_MethodAttributes_default;
		methAttr.executable     = true;
		methAttr.userExecutable = true;
		methAttr.description    = UA_LOCALIZEDTEXT((char *)"",
												   strMethName.data());
		methAttr.displayName    = UA_LOCALIZEDTEXT((char *)"",
												   strMethName.data());
		// create callback
		UA_NodeId methNodeId;
		auto st = UA_Server_addMethodNode(this->m_server,
										  UA_NODEID_NULL,
										  parentTypeNodeId,
										  UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
										  UA_QUALIFIEDNAME (1, strMethName.data()),
										  methAttr,
										  &QUaServer::methodCallback,
										  metamethod.parameterCount(),
										  vectArgs.data(),
										  outputArgument ? 1 : 0,
										  outputArgument,
										  this, // context is server instance that has m_hashMethods
										  &methNodeId);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		// Define "StartPump" method mandatory
		st = UA_Server_addReference(this->m_server,
							        methNodeId,
							        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
							        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
							        true);
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		// store method with node id hash as key
		Q_ASSERT_X(!m_hashMethods.contains(methNodeId), 
			"QUaServer::addMetaMethods", 
			"Method already exists, callback will be overwritten.");
		m_hashMethods[methNodeId] = [i, this](void * objectContext, const UA_Variant * input, UA_Variant * output) {
			// get object instance that owns method
			QUaBaseObject * object = dynamic_cast<QUaBaseObject*>(static_cast<QObject*>(objectContext));
			Q_ASSERT_X(object, 
				"QUaServer::addMetaMethods", 
				"Cannot call method on invalid C++ object.");
			if (!object)
			{
				return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
			}
			// NOTE : had to cache the method's index in lambda capture because caching metamethod directly was not working
			//        in some cases the internal data of the metamethod was deleted which resulted in access violation
			auto metamethod = object->metaObject()->method(i);
			// convert input arguments to QVariants
			QVariantList varListArgs;
			QList<QGenericArgument> genListArgs;
			auto listTypeNames = metamethod.parameterTypes();
			for (int k = 0; k < metamethod.parameterCount(); k++)
			{
				QVariant varArg;
				auto metaType    = (QMetaType::Type)metamethod.parameterType(k);
				auto strTypeName = QString(listTypeNames.at(k));
				// NOTE : enums are QMetaType::UnknownType
				Q_ASSERT_X(metaType != QMetaType::UnknownType ||
					this->m_hashEnums.contains(strTypeName),
					"QUaServer::addMetaMethods",
					"Argumant type is not registered. Try using qRegisterMetaType.");
				if (metaType == QMetaType::UnknownType &&
				   !this->m_hashEnums.contains(strTypeName))
				{
					return (UA_StatusCode)UA_STATUSCODE_BADINTERNALERROR;
				}
				if (strTypeName.contains("QList", Qt::CaseInsensitive))
				{
					varArg = QUaTypesConverter::uaVariantToQVariantArray(input[k], 
						QUaTypesConverter::ArrayType::QList);
				}
				else if (strTypeName.contains("QVector", Qt::CaseInsensitive))
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
					QMetaType::typeName(varListArgs[k].userType()),
					const_cast<void*>(varListArgs[k].constData())
				));
			}
			// create return QVariant
			int retType = metamethod.returnType();
			// NOTE : enums are QMetaType::UnknownType
			Q_ASSERT_X(retType != QMetaType::UnknownType ||
				this->m_hashEnums.contains(metamethod.typeName()),
				"QUaServer::addMetaMethods",
				"Return type is not registered. Try using qRegisterMetaType.");
			if (retType == QMetaType::UnknownType)
			{
				return (UA_StatusCode)UA_STATUSCODE_BADINTERNALERROR;
			}
			QVariant returnValue(retType, static_cast<void*>(NULL));
			QGenericReturnArgument returnArgument(
				metamethod.typeName(),
				const_cast<void*>(returnValue.constData())
			);
			// call metamethod
			bool ok = metamethod.invoke(
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
				// TODO : cleanup? UA_Variant_deleteMembers(&tmpVar)
				*output = tmpVar;
			}
			// return success status
			return (UA_StatusCode)UA_STATUSCODE_GOOD;
		};
	}
}

UA_NodeId QUaServer::createInstance(const QMetaObject & metaObject, QUaNode * parentNode, const QString &strNodeId/* = ""*/)
{
	// check if OPC UA relevant
	if (!metaObject.inherits(&QUaNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::createInstance", "Unsupported base class. It must derive from QUaNode");
		return UA_NODEID_NULL;
	}
	Q_ASSERT(!UA_NodeId_isNull(&parentNode->m_nodeId));
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// check if inherits BaseEventType, in which case this method cannot be used
	if (metaObject.inherits(&QUaBaseEvent::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::createInstance", "Cannot use createInstance to create Events. Use createEvent method instead");
		return UA_NODEID_NULL;
	}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// try to get typeNodeId, if null, then register it
	QString   strClassName = QString(metaObject.className());
	UA_NodeId typeNodeId   = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (UA_NodeId_isNull(&typeNodeId))
	{
		this->registerType(metaObject);
		typeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	}
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));
	// adapt parent relation with child according to parent type
	UA_NodeId referenceTypeId = QUaServer::getReferenceTypeId(*parentNode->metaObject(), metaObject);
	// set qualified name, default is class name
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name           = QUaTypesConverter::uaStringFromQString(metaObject.className());
	// check if requested node id defined
	QString strReqNodeId = strNodeId.trimmed();
	UA_NodeId reqNodeId = UA_NODEID_NULL;
	if (!strReqNodeId.isEmpty())
	{
		reqNodeId = QUaTypesConverter::nodeIdFromQString(strReqNodeId);
	}
	// check if requested node id exists
	UA_NodeId outNodeId;
	auto st = UA_Server_readNodeId(m_server, reqNodeId, &outNodeId);
	Q_ASSERT_X(st == UA_STATUSCODE_BADNODEIDUNKNOWN, "QUaServer::createInstance", "Requested NodeId already exists");
	if (st != UA_STATUSCODE_BADNODEIDUNKNOWN)
	{
		UA_NodeId_clear(&reqNodeId);
		UA_NodeId_clear(&outNodeId);
		UA_QualifiedName_clear(&browseName);
		return UA_NODEID_NULL;
	}
	// check if variable or object
	// NOTE : a type is considered to inherit itself (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
	UA_NodeId nodeIdNewInstance;
	if (metaObject.inherits(&QUaBaseVariable::staticMetaObject))
	{
		UA_VariableAttributes vAttr = UA_VariableAttributes_default;
		// [NOTE] do not set rank or arrayDimensions because they are permanent
		//        is better to just set array dimensions on Variant value and leave rank as ANY
		vAttr.valueRank = UA_VALUERANK_ANY;
		// add variable
		auto st = UA_Server_addVariableNode(m_server,
                                            reqNodeId,            // requested nodeId
                                            parentNode->m_nodeId, // parent
                                            referenceTypeId,      // parent relation with child
                                            browseName,
                                            typeNodeId, 
                                            vAttr, 
                                            nullptr,             // context
                                            &nodeIdNewInstance); // set new nodeId to new instance
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	else
	{
		Q_ASSERT(metaObject.inherits(&QUaBaseObject::staticMetaObject));
		UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		// add object
		auto st = UA_Server_addObjectNode(m_server,
                                          reqNodeId,            // requested nodeId
                                          parentNode->m_nodeId, // parent
                                          referenceTypeId,      // parent relation with child
                                          browseName,
                                          typeNodeId,
                                          oAttr, 
                                          nullptr,             // context
                                          &nodeIdNewInstance); // set new nodeId to new instance
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	// clean up
	UA_NodeId_clear(&reqNodeId);
	UA_NodeId_clear(&outNodeId);
	UA_NodeId_clear(&typeNodeId);
	UA_NodeId_clear(&referenceTypeId);
	UA_QualifiedName_clear(&browseName);

	// trigger reference added, model change event, so client (UaExpert) auto refreshes tree
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	Q_CHECK_PTR(m_changeEvent);
	// add reference added change to buffer
	this->addChange({
		parentNode->nodeId(),
		parentNode->typeDefinitionNodeId(),
		QUaChangeVerb::ReferenceAdded // UaExpert does not recognize QUaChangeVerb::NodeAdded
	});	
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// return new instance node id
	return nodeIdNewInstance;
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

UA_NodeId QUaServer::createEvent(const QMetaObject & metaObject, const UA_NodeId &nodeIdOriginator, const QStringList * defaultProperties)
{
	// check if derives from event
	if (!metaObject.inherits(&QUaBaseEvent::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::createEvent", "Unsupported event class. It must derive from QUaBaseEvent");
		return UA_NODEID_NULL;
	}
	// try to get typeEvtId, if null, then register it
	QString   strClassName = QString(metaObject.className());
	UA_NodeId typeEvtId    = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (UA_NodeId_isNull(&typeEvtId))
	{
		this->registerType(metaObject);
		typeEvtId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	}
	Q_ASSERT(!UA_NodeId_isNull(&typeEvtId));
	// set originator node id temporarily
	m_newEventOriginatorNodeId  = &nodeIdOriginator;
	m_newEventDefaultProperties = defaultProperties;
	// create event instance
	UA_NodeId nodeIdNewEvent;
	auto st = UA_Server_createEvent(m_server, typeEvtId, &nodeIdNewEvent);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return new instance node id
	return nodeIdNewEvent;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

void QUaServer::bindCppInstanceWithUaNode(QUaNode * nodeInstance, UA_NodeId & nodeId)
{
	Q_CHECK_PTR(nodeInstance);
	Q_ASSERT(!UA_NodeId_isNull(&nodeId));
	// set c++ instance as context
	UA_Server_setNodeContext(m_server, nodeId, (void**)(&nodeInstance));
	// set node id to c++ instance
	nodeInstance->m_nodeId = nodeId;
}

void QUaServer::registerEnum(const QString & strEnumName, const QUaEnumMap& enumMap, const QString & strNodeId)
{
	// check if already exists
	if (m_hashEnums.contains(strEnumName))
	{
		return;
	}
	// get c-compatible name
	QByteArray byteEnumName = strEnumName.toUtf8();
	char * charEnumName     = byteEnumName.data();
	// register enum as new ua data type
	UA_DataTypeAttributes ddaatt = UA_DataTypeAttributes_default;
	ddaatt.description = UA_LOCALIZEDTEXT((char*)(""), charEnumName);
	ddaatt.displayName = UA_LOCALIZEDTEXT((char*)(""), charEnumName);

	// check if requested node id defined
	QString strReqNodeId = strNodeId.trimmed();
	UA_NodeId reqNodeId  = strReqNodeId.isEmpty() ? UA_NODEID_NULL : QUaTypesConverter::nodeIdFromQString(strReqNodeId);
	// check if requested node id exists
	UA_NodeId outNodeId;
	auto st = UA_Server_readNodeId(m_server, reqNodeId, &outNodeId);
	Q_ASSERT_X(st == UA_STATUSCODE_BADNODEIDUNKNOWN, "QUaServer::registerEnum", "Requested NodeId already exists");
	if (st != UA_STATUSCODE_BADNODEIDUNKNOWN)
	{
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return;
	}
	// if null, then assign one because is feaking necessary
	// https://github.com/open62541/open62541/issues/2584
	if (UA_NodeId_isNull(&reqNodeId))
	{
		// [IMPORTANT] : _ALLOC version is necessary
		reqNodeId = UA_NODEID_STRING_ALLOC(1, charEnumName); 
	}	
	st = UA_Server_addDataTypeNode(m_server,
								   reqNodeId,
   								   UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMERATION),
   								   UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
   								   UA_QUALIFIEDNAME (1, charEnumName),
   								   ddaatt,
   								   NULL,
                                   NULL);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// add new enum strings (enum options)
	UA_VariableAttributes pattr = UA_VariableAttributes_default;
	pattr.description = UA_LOCALIZEDTEXT((char*)(""), (char*)("EnumStrings"));
	pattr.displayName = UA_LOCALIZEDTEXT((char*)(""), (char*)("EnumStrings"));
	pattr.dataType    = UA_TYPES[UA_TYPES_LOCALIZEDTEXT].typeId;
	UA_UInt32 arrayDimensions[1] = { 0 };
	pattr.valueRank              = 1; 
	pattr.arrayDimensionsSize    = 1;
	pattr.arrayDimensions        = arrayDimensions;
	// create vector of enum values
	QVector<QOpcUaEnumValue> vectEnumValues;
	QMapIterator<QUaEnumKey, QUaEnumEntry> i(enumMap);
	while (i.hasNext()) 
	{
		i.next();
		vectEnumValues.append({
			(UA_Int64)i.key(),
			UA_LOCALIZEDTEXT((char*)"", (char*)i.value().strDisplayName.data()),
			UA_LOCALIZEDTEXT((char*)"", (char*)i.value().strDescription.data())
			});
	}
	st = QUaServer::addEnumValues(m_server, &reqNodeId, vectEnumValues.count(), vectEnumValues.data());
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// finally append to map
	m_hashEnums.insert(strEnumName, reqNodeId);
}

bool QUaServer::isEnumRegistered(const QString & strEnumName) const
{
	return m_hashEnums.contains(strEnumName);
}

// NOTE : expensive, creates a copy
QUaEnumMap QUaServer::enumMap(const QString & strEnumName) const
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
	UA_EnumValueType *enumArr = static_cast<UA_EnumValueType *>(enumValues.data);
	for (size_t i = 0; i < enumValues.arrayLength; i++)
	{
		UA_EnumValueType * enumVal = &enumArr[i];
		QString strDisplayName = QUaTypesConverter::uaVariantToQVariantScalar<QString, UA_LocalizedText>(&enumVal->displayName);
		QString strDescription = QUaTypesConverter::uaVariantToQVariantScalar<QString, UA_LocalizedText>(&enumVal->description);
		retMap.insert(enumVal->value, { strDisplayName.toUtf8(), strDescription.toUtf8() });
	}
	// clean up
	UA_Variant_clear(&enumValues);
	// return
	return retMap;
}

void QUaServer::setEnumMap(const QString & strEnumName, const QUaEnumMap & enumMap)
{
	// if it does not exist, create it with one entry
	if (!m_hashEnums.contains(strEnumName))
	{
		this->registerEnum(strEnumName, enumMap);
		return;
	}
	// else update enum
	auto enumNodeId = m_hashEnums[strEnumName];
	this->updateEnum(enumNodeId, enumMap);
}

// NOTE : expensive because it needs to copy old array into new, update new and delete old array,
//        to insert multiple elements at once use QUaServer::updateEnum
void QUaServer::updateEnumEntry(const QString &strEnumName, const QUaEnumKey &enumValue, const QUaEnumEntry &enumEntry)
{
	// if it does not exist, create it with one entry
	if (!m_hashEnums.contains(strEnumName))
	{
		this->registerEnum(strEnumName, { {enumValue, enumEntry} });
		return;
	}
	// else update enum
	auto enumNodeId = m_hashEnums[strEnumName];
	// get old map
	auto mapValues = this->enumMap(strEnumName);
	// update old map
	mapValues[enumValue] = enumEntry;
	this->updateEnum(enumNodeId, mapValues);
}

// NOTE : expensive because it needs to copy old array into new, update new and delete old array,
//        to insert multiple elements at once use QUaServer::updateEnum with empty argument
void QUaServer::removeEnumEntry(const QString &strEnumName, const QUaEnumKey &enumValue)
{
	// if it does not exist, do nothing
	if (!m_hashEnums.contains(strEnumName))
	{
		return;
	}
	// else update enum
	auto enumNodeId = m_hashEnums[strEnumName];
	// get old map
	auto mapValues = this->enumMap(strEnumName);
	// update old map
	mapValues.remove(enumValue);
	this->updateEnum(enumNodeId, mapValues);
}

// NOTE : need to cleanup result after calling this method
UA_NodeId QUaServer::enumValuesNodeId(const UA_NodeId & enumNodeId) const
{
	// make ua browse
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
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
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);
	// return
	return valuesNodeId;
}

UA_Variant QUaServer::enumValues(const UA_NodeId &enumNodeId) const
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

void QUaServer::updateEnum(const UA_NodeId & enumNodeId, const QUaEnumMap & mapEnum)
{
	// get enum values as ua variant
	auto enumValuesNodeId = this->enumValuesNodeId(enumNodeId);
	auto enumValues       = this->enumValues(enumNodeId);
	// delete old ua enum
	UA_Variant_clear(&enumValues);
	// re-create array of enum values
	UA_EnumValueType * valueEnum = nullptr;
	if (mapEnum.count() > 0)
	{
		valueEnum = (UA_EnumValueType *)UA_malloc(sizeof(UA_EnumValueType) * mapEnum.count());
	}
	auto listKeys = mapEnum.keys();
	for (int i = 0; i < mapEnum.count(); i++)
	{
		UA_init(&valueEnum[i], &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
		valueEnum[i].value = (UA_Int64)listKeys.at(i);
		QUaTypesConverter::uaVariantFromQVariantScalar
			<UA_LocalizedText, QString>
			(mapEnum[listKeys.at(i)].strDisplayName, &valueEnum[i].displayName);
		QUaTypesConverter::uaVariantFromQVariantScalar
			<UA_LocalizedText, QString>
			(mapEnum[listKeys.at(i)].strDescription, &valueEnum[i].description);
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

void QUaServer::registerReference(const QUaReference & ref)
{
	// first check if already registered
	if (m_hashRefs.contains(ref))
	{
		return;
	}
	// get namea and stuff
	QByteArray byteForwardName = ref.strForwardName.toUtf8();
	QByteArray byteInverseName = ref.strInverseName.toUtf8();
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name = QUaTypesConverter::uaStringFromQString(ref.strForwardName);
	// setup new ref type attributes
	UA_ReferenceTypeAttributes refattr = UA_ReferenceTypeAttributes_default;
	refattr.displayName = UA_LOCALIZEDTEXT((char*)(""), byteForwardName.data());
	refattr.inverseName = UA_LOCALIZEDTEXT((char*)(""), byteInverseName.data());
	UA_NodeId outNewNodeId;
	auto st = UA_Server_addReferenceTypeNode(
		m_server,
		UA_NODEID_NULL,
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
	UA_QualifiedName_clear(&browseName);
	// add to hash to complete registration
	m_hashRefs.insert(ref, outNewNodeId);
}

QUaFolderObject * QUaServer::objectsFolder() const
{
	return m_pobjectsFolder;
}

QUaNode * QUaServer::nodeById(const QString & strNodeId)
{
	UA_NodeId nodeId = QUaTypesConverter::nodeIdFromQString(strNodeId);
	QUaNode * node = QUaNode::getNodeContext(nodeId, m_server);
	UA_NodeId_clear(&nodeId);
	return node;
}

QUaNode * QUaServer::browsePath(const QStringList & strBrowsePath) const
{
	if (strBrowsePath.count() <= 0)
	{
		return nullptr;
	}
	QString strFirst = strBrowsePath.first();
	// check if first is ObjectsFolder
	if (strFirst.compare(this->objectsFolder()->browseName(), Qt::CaseSensitive) == 0)
	{
		return this->objectsFolder()->browsePath(strBrowsePath.mid(1));
	}
	// then check if first is a child of ObjectsFolder
	auto listChildren = this->objectsFolder()->browseChildren();
	for (int i = 0; i < listChildren.count(); i++)
	{
		auto child = listChildren.at(i);
		if (strFirst.compare(child->browseName(), Qt::CaseSensitive) == 0)
		{
			return child->browsePath(strBrowsePath.mid(1));
		}
	}
	// if not, then not supported
	return nullptr;
}

bool QUaServer::anonymousLoginAllowed() const
{
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	AccessControlContext *context = static_cast<AccessControlContext*>(config->accessControl.context);
	return context->allowAnonymous;
}

void QUaServer::setAnonymousLoginAllowed(const bool & anonymousLoginAllowed) const
{
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	AccessControlContext *context = static_cast<AccessControlContext*>(config->accessControl.context);
	context->allowAnonymous = anonymousLoginAllowed;
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
	if (strUserName.isEmpty())
	{
		return false;
	}
	return m_hashUsers.contains(strUserName);
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
		if (childMetaObject.inherits(&QUaBaseObject::staticMetaObject))
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT);
		}
		else if (childMetaObject.inherits(&QUaBaseDataVariable::staticMetaObject))
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


