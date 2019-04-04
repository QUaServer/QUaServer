#include "quaserver.h"

#include <QMetaProperty>
#include <QTimer>

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

// [STATIC]
QMap<UA_Server*, QUaServer*> QUaServer::m_mapServers;

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
	return srv->m_hashConstructors[*typeNodeId](nodeId, nodeContext);
}

void QUaServer::uaDestructor(UA_Server       * server, 
	                         const UA_NodeId * sessionId, 
	                         void            * sessionContext, 
	                         const UA_NodeId * typeNodeId, 
	                         void            * typeNodeContext, 
	                         const UA_NodeId * nodeId, 
	                         void            ** nodeContext)
{
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
		return;
	}
	// if we reach here it means case 2), so deleteLater (when nodeId not in store anymore)
	// so we avoid calling UA_Server_deleteNode in ~QUaNode
	node->deleteLater();
	Q_UNUSED(st);
}

UA_StatusCode QUaServer::uaConstructor(QUaServer         * server,
	                                   const UA_NodeId   * nodeId, 
	                                   void             ** nodeContext,
	                                   const QMetaObject & metaObject)
{
	// get parent node id
	UA_NodeId parentNodeId       = QUaNode::getParentNodeId(*nodeId, server->m_server);
	UA_NodeId directParentNodeId = parentNodeId;
	Q_ASSERT(!UA_NodeId_isNull(&parentNodeId));
	// check if constructor from explicit instance creation or type registration
	// this is done by checking that parent is different from object ot variable instance
	UA_NodeClass outNodeClass;
	UA_NodeId lastParentNodeId;
	QUaNode * parentContext = nullptr;
	while (true)
	{
		// ignore if new instance is due to new type registration
		UA_Server_readNodeClass(server->m_server, parentNodeId, &outNodeClass);
		if(outNodeClass != UA_NODECLASS_OBJECT && outNodeClass != UA_NODECLASS_VARIABLE)
		{
			// remove from deferred constructors
			server->m_hashDeferredConstructors.remove(*nodeId);
			return UA_STATUSCODE_GOOD;
		}
		lastParentNodeId = parentNodeId;
		parentContext = QUaNode::getNodeContext(lastParentNodeId, server);
		// check if parent node is bound
		if (parentContext)
		{
			break;
		}
		parentNodeId = QUaNode::getParentNodeId(parentNodeId, server->m_server);
		// exit if null
		if (UA_NodeId_isNull(&parentNodeId))
		{
			break;
		}
	}
	// NOTE : at this point parentNodeId could be null, so we need lastParentNodeId which is guaranteed not to be null
	// if parentContext is valid, then we can call deferred constructors waiting for *nodeId and continue with execution
	if (!parentContext)
	{
		server->m_hashDeferredConstructors[lastParentNodeId].append([server, nodeId, nodeContext, metaObject]() {
			QUaServer::uaConstructor(server, nodeId, nodeContext, metaObject);
		});
		return UA_STATUSCODE_GOOD;
	}
	else
	{
		for (int i = 0; i < server->m_hashDeferredConstructors[*nodeId].count(); i++)
		{
			server->m_hashDeferredConstructors[*nodeId][i]();
		}
		server->m_hashDeferredConstructors.remove(*nodeId);
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
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	// need to bind again using the official (void ** nodeContext) of the UA constructor
	// because we set context on C++ instantiation, but later the UA library overwrites it 
	// after calling the UA constructor
	*nodeContext = (void*)newInstance;
	newInstance->m_nodeId = *nodeId;
	// need to set parent if direct parent is already bound bacaus eits constructor has already been called
	parentNodeId = QUaNode::getParentNodeId(*nodeId, server->m_server);
	if (UA_NodeId_equal(&parentNodeId, &lastParentNodeId))
	{
		newInstance->setParent(parentContext);
		newInstance->setObjectName(QUaNode::getBrowseName(*nodeId, server));
		// emit child added to parent
		emit parentContext->childAdded(newInstance);
	}
	// success
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

UA_StatusCode QUaServer::createEnumValue(const QOpcUaEnumValue * enumVal, UA_ExtensionObject * outExtObj)
{
	if (!outExtObj)
	{
		return UA_STATUSCODE_BADOUTOFMEMORY;
	}
	outExtObj->encoding               = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
	outExtObj->content.encoded.typeId = UA_NODEID_NUMERIC(0, 8251); // Default Binary (in open62541 UA_NS0ID_DEFAULTBINARY == 3062)
	UA_ByteString_allocBuffer(&outExtObj->content.encoded.body, 65000);
	UA_Byte * p_posExtObj = outExtObj->content.encoded.body.data;
	const UA_Byte * p_endExtObj = &outExtObj->content.encoded.body.data[65000];
	// encode enum value
	UA_StatusCode st = UA_STATUSCODE_GOOD;
	st |= UA_encodeBinary(&enumVal->Value      , &UA_TYPES[UA_TYPES_INT64]        , &p_posExtObj, &p_endExtObj, NULL, NULL);
	st |= UA_encodeBinary(&enumVal->DisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &p_posExtObj, &p_endExtObj, NULL, NULL);
	st |= UA_encodeBinary(&enumVal->Description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &p_posExtObj, &p_endExtObj, NULL, NULL);
	if (st != UA_STATUSCODE_GOOD)
	{
		return st;
	}
	size_t p_extobj_encOffset = (uintptr_t)(p_posExtObj - outExtObj->content.encoded.body.data);
	outExtObj->content.encoded.body.length = p_extobj_encOffset;
	UA_Byte * p_extobj_newBody = (UA_Byte *)UA_malloc(p_extobj_encOffset);
	if (!p_extobj_newBody)
	{
		return UA_STATUSCODE_BADOUTOFMEMORY;
	}
	memcpy(p_extobj_newBody, outExtObj->content.encoded.body.data, p_extobj_encOffset);
	UA_Byte * p_extobj_madatory_oldBody  = outExtObj->content.encoded.body.data;
	outExtObj->content.encoded.body.data = p_extobj_newBody;
	UA_free(p_extobj_madatory_oldBody);
	// success
	return UA_STATUSCODE_GOOD;
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
	attr.arrayDimensions         = (UA_UInt32 *)UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]);
	if (!attr.arrayDimensions)
	{
		return UA_STATUSCODE_BADOUTOFMEMORY;
	}
	attr.arrayDimensions[0] = 0;
	attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMVALUETYPE);
	// create array of enum values
	UA_ExtensionObject * arr_extobjs = (UA_ExtensionObject *)UA_malloc(sizeof(UA_ExtensionObject) * numEnumValues);
	for (size_t i = 0; i < numEnumValues; i++)
	{
		retVal |= createEnumValue(&enumValues[i], &arr_extobjs[i]);
		Q_ASSERT(retVal == UA_STATUSCODE_GOOD);
	}
	// create variant with array of enum values
	UA_Variant_setArray(&attr.value, arr_extobjs, (UA_Int32)numEnumValues, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
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
	// cleanup
	UA_Array_delete(attr.arrayDimensions, 1, &UA_TYPES[UA_TYPES_UINT32]);
	for (size_t i = 0; i < numEnumValues; i++)
	{
		UA_ExtensionObject_deleteMembers(&arr_extobjs[i]);
	}
	UA_free(arr_extobjs);
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
	AccessControlContext *context = (AccessControlContext*)ac->context;

	// NOTE : custom code : get server instance
	QUaServer *qServer = QUaServer::m_mapServers.value(server);
	Q_CHECK_PTR(qServer);

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
		Q_ASSERT(!qServer->m_hashSessions.contains(*sessionId));
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
		UA_Boolean match = false;

		// NOTE : custom code : check user and password
		const QString userName = QString::fromUtf8((char*)userToken->userName.data, (int)userToken->userName.length);
		const QString password = QString::fromUtf8((char*)userToken->password.data, (int)userToken->password.length);	
		QHashIterator<QString, QString> i(qServer->m_hashUsers);
		while (i.hasNext()) 
		{
			i.next();
			auto user = i.key();
			auto pass = i.value();
			if (user.compare(userName, Qt::CaseInsensitive) == 0 &&
				pass.compare(password, Qt::CaseInsensitive) == 0)
			{
				match = true;
				break;
			}
		}
		
		if (!match)
			return UA_STATUSCODE_BADUSERACCESSDENIED;

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
	// get server
	QUaServer *qServer = QUaServer::m_mapServers.value(server);
	Q_CHECK_PTR(qServer);
	// remove session form hash
	qServer->m_hashSessions.remove(*sessionId);
}

UA_UInt32 QUaServer::getUserRightsMask(UA_Server        *server,
	                                   UA_AccessControl *ac,
	                                   const UA_NodeId  *sessionId,
	                                   void             *sessionContext,
	                                   const UA_NodeId  *nodeId,
	                                   void             *nodeContext) {
	// get server
	QUaServer *qServer = QUaServer::m_mapServers.value(server);
	Q_CHECK_PTR(qServer);
	Q_ASSERT(qServer->m_hashSessions.contains(*sessionId));
	// get user
	QString strUserName = qServer->m_hashSessions.value(*sessionId);
	// check if user still exists
	if (!strUserName.isEmpty() && !qServer->userExists(strUserName))
	{
		// TODO : waint until officially supported
		// https://github.com/open62541/open62541/issues/2617
		//auto st = UA_Server_closeSession(server, sessionId);
		//Q_ASSERT(st == UA_STATUSCODE_GOOD);
		return (UA_UInt32)0;
	}
	// if node from user tree then call user implementation
	QUaNode * node = QUaNode::getNodeContext(*nodeId, server);
	if (node)
	{
		return node->userWriteMask(strUserName).intValue;
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
	// get server
	QUaServer *qServer = QUaServer::m_mapServers.value(server);
	Q_CHECK_PTR(qServer);
	Q_ASSERT(qServer->m_hashSessions.contains(*sessionId));
	// get user
	QString strUserName = qServer->m_hashSessions.value(*sessionId);
	// check if user still exists
	if (!strUserName.isEmpty() && !qServer->userExists(strUserName))
	{
		// TODO : waint until officially supported
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
		return variable->userAccessLevel(strUserName).intValue;
	}
	// else default
	return 0xFF;
}

UA_Boolean QUaServer::getUserExecutable(UA_Server        *server, 
		                                UA_AccessControl *ac,
		                                const UA_NodeId  *sessionId, 
		                                void             *sessionContext,
		                                const UA_NodeId  *methodId, 
		                                void             *methodContext)
{
	UA_NodeId parentNodeId = QUaNode::getParentNodeId(*methodId, server);
	if (!UA_NodeId_isNull(&parentNodeId))
	{
		void * context = QUaNode::getVoidContext(parentNodeId, server);
		if (context)
		{
			return QUaServer::getUserExecutableOnObject(
				server, 
				ac,
				sessionId, 
				sessionContext,
				methodId, 
				methodContext,
				&parentNodeId,
				context
			);
		}
	}
	return true;
}

UA_Boolean QUaServer::getUserExecutableOnObject(UA_Server        *server, 
		                                        UA_AccessControl *ac,
		                                        const UA_NodeId  *sessionId, 
		                                        void             *sessionContext,
		                                        const UA_NodeId  *methodId, 
		                                        void             *methodContext,
		                                        const UA_NodeId  *objectId, 
		                                        void             *objectContext)
{
	// get server
	QUaServer *qServer = QUaServer::m_mapServers.value(server);
	Q_CHECK_PTR(qServer);
	Q_ASSERT(qServer->m_hashSessions.contains(*sessionId));
	// get user
	QString strUserName = qServer->m_hashSessions.value(*sessionId);
	// check if user still exists
	if (!strUserName.isEmpty() && !qServer->userExists(strUserName))
	{
		// TODO : waint until officially supported
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
		return object->userExecutable(strUserName);
	}
	// else default
	return true;
}

void QUaServer::writeBuildInfo(UA_Server         *server, 
	                           const UA_NodeId    nodeId, 
	                           void *UA_RESTRICT  p,
	                           const UA_DataType *type)
{
	UA_ServerConfig * config = UA_Server_getConfig(server);
	UA_Variant var;
	UA_Variant_init(&var);
	UA_Variant_setScalar(&var, p, type);
	auto st = UA_Server_writeValue(server, nodeId, var);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

#ifndef UA_ENABLE_ENCRYPTION
// NOTE : cannot load cert later with UA_Server_updateCertificate because
//        it requires oldCertificate != NULL
QUaServer::QUaServer(const quint16    &intPort        /* = 4840*/, 
	                 const QByteArray &byteCertificate/* = QByteArray()*/, 
	                 QObject          *parent         /* = 0*/) 
	: QObject(parent)
{
	// convert cert if valid
	UA_ByteString cert;
	UA_ByteString * ptr = this->parseCertificate(byteCertificate, cert, m_byteCertificate);
	// create config with port and certificate
	UA_ServerConfig * config = UA_ServerConfig_new_minimal(intPort, ptr);
	Q_CHECK_PTR(config);
	this->m_server = UA_Server_new(config);
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
	UA_ServerConfig * config;
	// convert cert if valid (should contain public key)
	UA_ByteString cert;
	UA_ByteString * ptrCert = this->parseCertificate(byteCertificate, cert, m_byteCertificate);
	// convert private key if valid
	UA_ByteString priv;
	UA_ByteString * ptrPriv = this->parseCertificate(bytePrivateKey, priv, m_bytePrivateKey);
	// check if valid private key
	if (ptrPriv)
	{
		// create config with port, certificate and private key for encryption
		config = UA_ServerConfig_new_basic256sha256(intPort, ptrCert, ptrPriv, nullptr, 0, nullptr, 0);
	}
	else
	{
		// create config with port and certificate only (no encryption)
		config = UA_ServerConfig_new_minimal(intPort, ptrCert);
	}
	Q_CHECK_PTR(config);
	this->m_server = UA_Server_new(config);
	// setup server
	this->setupServer();
}
#endif

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
	UA_StatusCode st;
	m_running = false;
	// Add to hash of server pairs
	QUaServer::m_mapServers[this->m_server] = this;
	// Create "Objects" folder using special constructor
	// Part 5 - 8.2.4 : Objects
	auto objectsNodeId        = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	this->m_newNodeNodeId     = &objectsNodeId;
	this->m_newNodeMetaObject = &QUaFolderObject::staticMetaObject;
	m_pobjectsFolder = new QUaFolderObject(this);
	m_pobjectsFolder->setParent(this);
	m_pobjectsFolder->setObjectName("Objects");
	// register base types
	m_mapTypes.insert(QString(QUaBaseVariable::staticMetaObject.className())    , UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE));
	m_mapTypes.insert(QString(QUaBaseDataVariable::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE));
	m_mapTypes.insert(QString(QUaProperty::staticMetaObject.className())        , UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE));
	m_mapTypes.insert(QString(QUaBaseObject::staticMetaObject.className())      , UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE));
	m_mapTypes.insert(QString(QUaFolderObject::staticMetaObject.className())    , UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE));
	// set context for base types
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// register constructors for instantiable types
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), QUaBaseDataVariable::staticMetaObject);
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , QUaProperty::staticMetaObject);
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , QUaBaseObject::staticMetaObject);
	this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , QUaFolderObject::staticMetaObject);
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
		config->applicationDescription.applicationName.text.length
	);
	// get server app uri
	m_byteApplicationUri = QByteArray::fromRawData(
		(char*)config->applicationDescription.applicationUri.data,
		config->applicationDescription.applicationUri.length
	);

	// get server product name
	m_byteProductName = QByteArray::fromRawData(
		(char*)config->buildInfo.productName.data,
		config->buildInfo.productName.length
	);
	// get server product uri
	m_byteProductUri = QByteArray::fromRawData(
		(char*)config->buildInfo.productUri.data,
		config->buildInfo.productUri.length
	);
	// get server manufacturer name
	m_byteManufacturerName = QByteArray::fromRawData(
		(char*)config->buildInfo.manufacturerName.data,
		config->buildInfo.manufacturerName.length
	);
	// get server software version
	m_byteSoftwareVersion = QByteArray::fromRawData(
		(char*)config->buildInfo.softwareVersion.data,
		config->buildInfo.softwareVersion.length
	);
	// get server software version
	m_byteBuildNumber = QByteArray::fromRawData(
		(char*)config->buildInfo.buildNumber.data,
		config->buildInfo.buildNumber.length
	);
}

QUaServer::~QUaServer()
{
	// Remove from servers map
	m_mapServers.remove(this->m_server);
	// Cleanup library objects
	UA_ServerConfig * config = UA_Server_getConfig(m_server);
	UA_ServerConfig_delete(config);
	UA_Server_delete(this->m_server);
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
	// write parent node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO),
		(void*)&config->buildInfo,
		&UA_TYPES[UA_TYPES_BUILDINFO]
	);
	// write specific node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTNAME),
		(void*)&config->buildInfo.productName,
		&UA_TYPES[UA_TYPES_STRING]
	);
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
	// write parent node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO),
		(void*)&config->buildInfo,
		&UA_TYPES[UA_TYPES_BUILDINFO]
	);
	// write specific node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTURI),
		(void*)&config->buildInfo.productUri,
		&UA_TYPES[UA_TYPES_STRING]
	);
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
	// write parent node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO),
		(void*)&config->buildInfo,
		&UA_TYPES[UA_TYPES_BUILDINFO]
	);
	// write specific node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_MANUFACTURERNAME),
		(void*)&config->buildInfo.manufacturerName,
		&UA_TYPES[UA_TYPES_STRING]
	);
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
	// write parent node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO),
		(void*)&config->buildInfo,
		&UA_TYPES[UA_TYPES_BUILDINFO]
	);
	// write specific node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_SOFTWAREVERSION),
		(void*)&config->buildInfo.softwareVersion,
		&UA_TYPES[UA_TYPES_STRING]
	);
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
	// write parent node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO),
		(void*)&config->buildInfo,
		&UA_TYPES[UA_TYPES_BUILDINFO]
	);
	// write specific node
	QUaServer::writeBuildInfo(
		m_server,
		UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_BUILDNUMBER),
		(void*)&config->buildInfo.buildNumber,
		&UA_TYPES[UA_TYPES_STRING]
	);
}

void QUaServer::start()
{
	// NOTE : we must define port and other server params upon instantiation, 
	//        because rest of API assumes m_server is valid
	if (m_running)
	{
		return;
	}
	auto st = UA_Server_run_startup(this->m_server);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st)
	m_running = true;
	m_connection = QObject::connect(this, &QUaServer::iterateServer, this, [this]() {
		if (m_running) 
		{
			UA_Server_run_iterate(this->m_server, true);
			// iterate again
			emit this->iterateServer();
		}	
	}, Qt::QueuedConnection);
	// bootstrap iterations
	emit this->iterateServer();
}

void QUaServer::stop()
{
	m_running = false;
	QObject::disconnect(m_connection);
	UA_Server_run_shutdown(this->m_server);
}

bool QUaServer::isRunning()
{
	return m_running;
}

void QUaServer::registerType(const QMetaObject &metaObject)
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
			                                    UA_NODEID_NULL,                            // requested nodeId
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
			                                  UA_NODEID_NULL,                            // requested nodeId
			                                  m_mapTypes.value(strBaseClassName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)), // parent (object type)
			                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), // parent relation with child
			                                  browseName,
			                                  otAttr,
			                                  (void*)this,                               // context : server instance where type was registered
			                                  &newTypeNodeId);                           // new object type id
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
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

void QUaServer::registerEnum(const QMetaEnum & metaEnum)
{
	// compose enum name
	QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
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
	UA_NodeId newEnumNodeId = UA_NODEID_STRING_ALLOC(1, charEnumName); // [IMPORTANT] : _ALLOC version is necessary
	UA_StatusCode st = UA_Server_addDataTypeNode(m_server,
												newEnumNodeId,
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
	QVector<QByteArray> byteKeys;
	QVector<QOpcUaEnumValue> vectEnumValues;
	for (int i = 0; i < metaEnum.keyCount(); i++)
	{
		byteKeys.append(metaEnum.key(i));
		vectEnumValues.append({
			(UA_Int64)metaEnum.value(i),
			UA_LOCALIZEDTEXT((char*)"", byteKeys[i].data()),
			UA_LOCALIZEDTEXT((char*)"", (char*)"")
		});
	}
	st = QUaServer::addEnumValues(m_server, &newEnumNodeId, vectEnumValues.count(), vectEnumValues.data());
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// finally append to map
	m_hashEnums.insert(strEnumName, newEnumNodeId);
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
			QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
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
		// validate return type
		auto returnType  = (QMetaType::Type)metamethod.returnType();
		bool isSupported = QUaTypesConverter::isSupportedQType(returnType);
		Q_ASSERT_X(isSupported, "QUaServer::addMetaMethods", "Return type not supported in MetaMethod.");
		if (!isSupported)
		{
			continue;
		}
		// create return type
		UA_Argument   outputArgumentInstance;
		UA_Argument * outputArgument = nullptr;
		if (returnType != QMetaType::Void)
		{
			UA_Argument_init(&outputArgumentInstance);
			outputArgumentInstance.description = UA_LOCALIZEDTEXT((char *)"",
														  (char *)"Result Value");
			outputArgumentInstance.name        = QUaTypesConverter::uaStringFromQString((char *)"Result");
			outputArgumentInstance.dataType    = QUaTypesConverter::uaTypeNodeIdFromQType(returnType);
			outputArgumentInstance.valueRank   = UA_VALUERANK_SCALAR;
			outputArgument = &outputArgumentInstance;
		}
		// validate argument types and create them
		Q_ASSERT_X(metamethod.parameterCount() <= 10, "QUaServer::addMetaMethods", "No more than 10 arguments supported in MetaMethod.");
		if (metamethod.parameterCount() > 10)
		{
			continue;
		}
		QVector<UA_Argument> vectArgs;
		auto listArgNames = metamethod.parameterNames();
		Q_ASSERT(listArgNames.count() == metamethod.parameterCount());
		for (int k = 0; k < metamethod.parameterCount(); k++)
		{
			isSupported = QUaTypesConverter::isSupportedQType((QMetaType::Type)metamethod.parameterType(k));
			Q_ASSERT_X(isSupported, "QUaServer::addMetaMethods", "Argument type not supported in MetaMethod.");
			if (!isSupported)
			{
				break;
			}
			UA_Argument inputArgument;
			UA_Argument_init(&inputArgument);
			// create n-th argument
			inputArgument.description = UA_LOCALIZEDTEXT((char *)"", (char *)"Method Argument");
			inputArgument.name        = QUaTypesConverter::uaStringFromQString(listArgNames.at(k));
			inputArgument.dataType    = QUaTypesConverter::uaTypeNodeIdFromQType((QMetaType::Type)metamethod.parameterType(k));
			inputArgument.valueRank   = UA_VALUERANK_SCALAR;
			vectArgs.append(inputArgument);
		}
		if (!isSupported)
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
		Q_ASSERT_X(!m_hashMethods.contains(methNodeId), "QUaServer::addMetaMethods", "Method already exists, callback will be overwritten.");
		m_hashMethods[methNodeId] = [metamethod](void * objectContext, const UA_Variant * input, UA_Variant * output) {
			// get object instance that owns method
			QUaBaseObject * object = dynamic_cast<QUaBaseObject*>(static_cast<QObject*>(objectContext));
			Q_ASSERT_X(object, "QUaServer::addMetaMethods", "Cannot call method on invalid C++ object.");
			if (!object)
			{
				return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
			}
			// convert input arguments to QVariants
			QVariantList varListArgs;
			QList<QGenericArgument> genListArgs;
			for (int k = 0; k < metamethod.parameterCount(); k++)
			{
				varListArgs.append(QUaTypesConverter::uaVariantToQVariant(input[k]));
				genListArgs.append(QGenericArgument(
					QMetaType::typeName(varListArgs[k].userType()),
					const_cast<void*>(varListArgs[k].constData())
				));
			}
			// create return QVariant
			QVariant returnValue(QMetaType::type(metamethod.typeName()), static_cast<void*>(NULL));
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
			if ((QMetaType::Type)metamethod.returnType() != QMetaType::Void)
			{
				*output = QUaTypesConverter::uaVariantFromQVariant(returnValue);
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
		Q_ASSERT_X(false, "QUaServer::createInstance", "Unsupported base class");
		return UA_NODEID_NULL;
	}
	Q_ASSERT(!UA_NodeId_isNull(&parentNode->m_nodeId));
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
	UA_NodeId referenceTypeId = QUaServer::getReferenceTypeId(parentNode->metaObject()->className(), 
		                                                         metaObject.className());
	// set qualified name, default is class name
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name           = QUaTypesConverter::uaStringFromQString(metaObject.className());
	// check if requested node id defined
	QString strReqNodeId = strNodeId.trimmed();
	UA_NodeId reqNodeId  = strReqNodeId.isEmpty() ? UA_NODEID_NULL : QUaTypesConverter::nodeIdFromQString(strReqNodeId);
	// check if requested node id exists
	UA_NodeId outNodeId;
	auto st = UA_Server_readNodeId(m_server, reqNodeId, &outNodeId);
	Q_ASSERT_X(st == UA_STATUSCODE_BADNODEIDUNKNOWN, "QUaServer::createInstance", "Requested NodeId already exists");
	if (st != UA_STATUSCODE_BADNODEIDUNKNOWN)
	{
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
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
	// check no pending constructors
	Q_ASSERT(m_hashDeferredConstructors.count() == 0);
	// return new instance node id
	return nodeIdNewInstance;
}

void QUaServer::bindCppInstanceWithUaNode(QUaNode * nodeInstance, UA_NodeId & nodeId)
{
	Q_CHECK_PTR(nodeInstance);
	Q_ASSERT(!UA_NodeId_isNull(&nodeId));
	// set c++ instance as context
	UA_Server_setNodeContext(m_server, nodeId, (void**)(&nodeInstance));
	// set node id to c++ instance
	nodeInstance->m_nodeId = nodeId;
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
	// add to hash to complete registration
	m_hashRefs.insert(ref, outNewNodeId);
}

QUaFolderObject * QUaServer::objectsFolder()
{
	return m_pobjectsFolder;
}

QUaNode * QUaServer::nodeById(const QString & strNodeId)
{
	UA_NodeId nodeId = QUaTypesConverter::nodeIdFromQString(strNodeId);
	return QUaNode::getNodeContext(nodeId, m_server);
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

void QUaServer::addUser(const QString & strUserName, const QString & strPassword)
{
	if (strUserName.isEmpty())
	{
		return;
	}
	m_hashUsers[strUserName] = strPassword;
}

void QUaServer::removeUser(const QString & strUserName)
{
	if (strUserName.isEmpty())
	{
		return;
	}
	m_hashUsers.remove(strUserName);
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

UA_NodeId QUaServer::getReferenceTypeId(const QString & strParentClassName, const QString & strChildClassName)
{
	UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	// adapt parent relation with child according to parent type
	if (strParentClassName.compare(QUaFolderObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
	{
		referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	}
	else if (strParentClassName.compare(QUaBaseObject      ::staticMetaObject.className(), Qt::CaseInsensitive) == 0 ||
		     strParentClassName.compare(QUaBaseDataVariable::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
	{
		if (strChildClassName.compare(QUaFolderObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0 ||
			strChildClassName.compare(QUaBaseObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT);
		}
		else if (strChildClassName.compare(QUaBaseDataVariable::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
		}
		else if (strChildClassName.compare(QUaProperty::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
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


