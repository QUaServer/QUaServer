#include "qopcuaserver.h"

#include <QMetaProperty>

// [STATIC]
UA_StatusCode QOpcUaServer::uaConstructor(UA_Server       * server, 
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
	auto obj = static_cast<QOpcUaServer*>(typeNodeContext);
	Q_CHECK_PTR(obj);
	Q_ASSERT(obj->m_hashConstructors.contains(*typeNodeId));
	// get method from type constructors map and call it
	return obj->m_hashConstructors[*typeNodeId](nodeId, nodeContext);
}

UA_StatusCode QOpcUaServer::uaConstructor(QOpcUaServer      * server,
	                                      const UA_NodeId   * nodeId, 
	                                      void             ** nodeContext,
	                                      const QMetaObject & metaObject)
{
	// get parent node id
	UA_NodeId parentNodeId       = QOpcUaServerNode::getParentNodeId(*nodeId, server->m_server);
	UA_NodeId directParentNodeId = parentNodeId;
	Q_ASSERT(!UA_NodeId_isNull(&parentNodeId));
	// check if constructor from explicit instance creation or type registration
	// this is done by checking that parent is different from object ot variable instance
	UA_NodeClass outNodeClass;
	UA_NodeId lastParentNodeId;
	QOpcUaServerNode * parentContext = nullptr;
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
		parentContext = QOpcUaServerNode::getNodeContext(lastParentNodeId, server);
		// check if parent node is bound
		if (parentContext)
		{
			break;
		}
		parentNodeId = QOpcUaServerNode::getParentNodeId(parentNodeId, server->m_server);
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
			QOpcUaServer::uaConstructor(server, nodeId, nodeContext, metaObject);
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
	Q_ASSERT_X(metaObject.constructorCount() > 0, "QOpcUaServer::uaConstructor", "Failed instantiation. No matching Q_INVOKABLE constructor with signature CONSTRUCTOR(QOpcUaServer *server, const UA_NodeId &nodeId) found.");
	auto * pQObject    = metaObject.newInstance(Q_ARG(QOpcUaServer*, server), Q_ARG(UA_NodeId, *nodeId), Q_ARG(QMetaObject, metaObject));
	Q_ASSERT_X(pQObject, "QOpcUaServer::uaConstructor", "Failed instantiation. No matching Q_INVOKABLE constructor with signature CONSTRUCTOR(QOpcUaServer *server, const UA_NodeId &nodeId) found.");
	auto * newInstance = static_cast<QOpcUaServerNode*>(pQObject);
	Q_CHECK_PTR(newInstance);
	// need to bind again using the official (void ** nodeContext) of the UA constructor
	// because we set context on C++ instantiation, but later the UA library overwrites it 
	// after calling the UA constructor
	*nodeContext = (void*)newInstance;
	newInstance->m_nodeId = *nodeId;
	// need to set parent if direct parent is already bound bacaus eits constructor has already been called
	parentNodeId = QOpcUaServerNode::getParentNodeId(*nodeId, server->m_server);
	if (UA_NodeId_equal(&parentNodeId, &lastParentNodeId))
	{
		newInstance->setParent(parentContext);
		newInstance->setObjectName(QOpcUaServerNode::getBrowseName(*nodeId, server));
	}
	// success
	return UA_STATUSCODE_GOOD;
}

// [STATIC]
UA_StatusCode QOpcUaServer::methodCallback(UA_Server        * server,
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
	auto srv = static_cast<QOpcUaServer*>(methodContext);
	Q_CHECK_PTR(srv);
	Q_ASSERT(srv->m_hashMethods.contains(*methodId));
	// get method from node callbacks map and call it
	return srv->m_hashMethods[*methodId](objectContext, input, output);
}

bool QOpcUaServer::isNodeBound(const UA_NodeId & nodeId, UA_Server *server)
{
	auto ptr = QOpcUaServerNode::getNodeContext(nodeId, server);
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

QOpcUaServer::QOpcUaServer(QObject *parent) : QObject(parent)
{
	UA_StatusCode st;
	UA_ServerConfig *config  = UA_ServerConfig_new_default();
	this->m_server = UA_Server_new(config);
	m_running = false;
	// Create "Objects" folder using special constructor
	// Part 5 - 8.2.4 : Objects
	m_pobjectsFolder = new QOpcUaFolderObject(this, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), QOpcUaFolderObject::staticMetaObject);
	m_pobjectsFolder->setParent(this);
	m_pobjectsFolder->setObjectName("Objects");
	// register base types
	m_mapTypes.insert(QString(QOpcUaBaseVariable    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE)    );
	m_mapTypes.insert(QString(QOpcUaBaseDataVariable::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE));
	m_mapTypes.insert(QString(QOpcUaProperty        ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        );
	m_mapTypes.insert(QString(QOpcUaBaseObject      ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      );
	m_mapTypes.insert(QString(QOpcUaFolderObject    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          );
	// set context for base types
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// register constructors for instantiable types
    this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), QOpcUaBaseDataVariable::staticMetaObject);
    this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , QOpcUaProperty        ::staticMetaObject);
    this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , QOpcUaBaseObject      ::staticMetaObject);
    this->registerTypeLifeCycle(UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , QOpcUaFolderObject    ::staticMetaObject);
}

/* TODO : alternative constructor
UA_EXPORT UA_ServerConfig * UA_ServerConfig_new_minimal(UA_UInt16 portNumber, const UA_ByteString *certificate);
*/

void QOpcUaServer::start()
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
	m_connection = QObject::connect(this, &QOpcUaServer::iterateServer, this, [this]() {
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

void QOpcUaServer::stop()
{
	m_running = false;
	QObject::disconnect(m_connection);
	UA_Server_run_shutdown(this->m_server);
}

bool QOpcUaServer::isRunning()
{
	return m_running;
}

void QOpcUaServer::registerType(const QMetaObject &metaObject)
{
	// check if OPC UA relevant
	if (!metaObject.inherits(&QOpcUaServerNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QOpcUaServer::registerType", "Unsupported base class");
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
	browseName.name = QOpcUaTypesConverter::uaStringFromQString(strClassName);
	// check if base class is registered
	QString strBaseClassName = QString(metaObject.superClass()->className());
	if (!m_mapTypes.contains(strBaseClassName))
	{
		// recursive
		this->registerType(*metaObject.superClass());
	}
	Q_ASSERT_X(m_mapTypes.contains(strBaseClassName), "QOpcUaServer::registerType", "Base object type not registered.");
	// check if variable or object
	if (metaObject.inherits(&QOpcUaBaseDataVariable::staticMetaObject))
	{
		// create variable type attributes
		UA_VariableTypeAttributes vtAttr = UA_VariableTypeAttributes_default;
		// set node attributes		  
		QByteArray byteDisplayName       = strClassName.toUtf8();
		vtAttr.displayName               = UA_LOCALIZEDTEXT((char*)"en-US", byteDisplayName.data());
		QByteArray byteDescription       = QString("").toUtf8();
		vtAttr.description               = UA_LOCALIZEDTEXT((char*)"en-US", byteDescription.data());
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
		Q_ASSERT(metaObject.inherits(&QOpcUaBaseObject::staticMetaObject));
		// create object type attributes
		UA_ObjectTypeAttributes otAttr = UA_ObjectTypeAttributes_default;
		// set node attributes		  
		QByteArray byteDisplayName     = strClassName.toUtf8();
		otAttr.displayName             = UA_LOCALIZEDTEXT((char*)"en-US", byteDisplayName.data());
		QByteArray byteDescription     = QString("").toUtf8();
		otAttr.description             = UA_LOCALIZEDTEXT((char*)"en-US", byteDescription.data());
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
	// register meta-properties
	this->addMetaProperties(metaObject);
	// register meta-methods (only if object class, or NOT variable class)
	if (!metaObject.inherits(&QOpcUaBaseDataVariable::staticMetaObject))
	{
		this->addMetaMethods(metaObject);
	}
}

void QOpcUaServer::registerTypeLifeCycle(const UA_NodeId &typeNodeId, const QMetaObject &metaObject)
{
    this->registerTypeLifeCycle(&typeNodeId, metaObject);
}

void QOpcUaServer::registerTypeLifeCycle(const UA_NodeId * typeNodeId, const QMetaObject & metaObject)
{
	Q_CHECK_PTR(typeNodeId);
	Q_ASSERT(!UA_NodeId_isNull(typeNodeId));
	if (UA_NodeId_isNull(typeNodeId))
	{
		return;
	}
	// add constructor
	Q_ASSERT_X(!m_hashConstructors.contains(*typeNodeId), "QOpcUaServer::registerType", "Constructor for type already exists.");
	// NOTE : we need constructors to be lambdas in order to cache metaobject in capture
	//        because type context is already the server instance where type was registered
	//        so we can differentiate the server instance in the static ::uaConstructor callback
	//        TLDR; to support multiple server instances in an application
	m_hashConstructors[*typeNodeId] = [metaObject, this](const UA_NodeId *instanceNodeId, void ** nodeContext) {
		// call static method
		return QOpcUaServer::uaConstructor(this, instanceNodeId, nodeContext, metaObject);
	};

	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &QOpcUaServer::uaConstructor;
	
	// TODO : destructor
	//lifecycle.destructor  = ;
	
	auto st = UA_Server_setNodeTypeLifecycle(m_server, *typeNodeId, lifecycle);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st)
}

void QOpcUaServer::addMetaProperties(const QMetaObject & parentMetaObject)
{
	QString   strParentClassName = QString(parentMetaObject.className());
	UA_NodeId parentTypeNodeId   = m_mapTypes.value(strParentClassName, UA_NODEID_NULL);
	Q_ASSERT(!UA_NodeId_isNull(&parentTypeNodeId));
	// loop meta properties and find out which ones inherit from
	int propCount = parentMetaObject.propertyCount();
	for (int i = parentMetaObject.propertyOffset(); i < propCount; i++)
	{
		// check if available in meta-system
		QMetaProperty metaproperty = parentMetaObject.property(i);
		if (!QMetaType::metaObjectForType(metaproperty.userType()))
		{
			continue;
		}
		// check if OPC UA relevant type
		const QMetaObject propMetaObject = *QMetaType::metaObjectForType(metaproperty.userType());
		if (!propMetaObject.inherits(&QOpcUaServerNode::staticMetaObject))
		{
			continue;
		}
		// check if prop inherits from parent
		Q_ASSERT_X(!propMetaObject.inherits(&parentMetaObject), "QOpcUaServer::addMetaProperties", "Qt MetaProperty type cannot inherit from Class.");
		if (propMetaObject.inherits(&parentMetaObject))
		{
			continue;
		}
		// check if prop type registered, register of not
		QString   strPropName      = QString(metaproperty.name());
		QString   strPropClassName = QString(propMetaObject.className());
		UA_NodeId propTypeNodeId   = m_mapTypes.value(strPropClassName, UA_NODEID_NULL);
		if (UA_NodeId_isNull(&propTypeNodeId))
		{
			this->registerType(propMetaObject);
			propTypeNodeId = m_mapTypes.value(strPropClassName, UA_NODEID_NULL);
		}
		Q_ASSERT(!UA_NodeId_isNull(&propTypeNodeId));
		// parent relation with child
		UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
		// set qualified name, default is class name
		UA_QualifiedName browseName;
		browseName.namespaceIndex = 1;
		browseName.name           = QOpcUaTypesConverter::uaStringFromQString(strPropName);
		// display name
		QByteArray bytePropName = strPropName.toUtf8();
		UA_LocalizedText displayName = UA_LOCALIZEDTEXT((char*)"en-US", bytePropName.data());
		// check if variable or object
		// NOTE : a type is considered to inherit itself sometimes does not work (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
		UA_NodeId tempNodeId;
		if (propMetaObject.inherits(&QOpcUaBaseVariable::staticMetaObject))
		{
			if (propMetaObject.inherits(&QOpcUaProperty::staticMetaObject))
			{
				referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
			}
			UA_VariableAttributes vAttr = UA_VariableAttributes_default;
			vAttr.displayName = displayName;
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
			// NOTE : not working ! Q_ASSERT(propMetaObject.inherits(&QOpcUaBaseObject::staticMetaObject));
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

void QOpcUaServer::addMetaMethods(const QMetaObject & parentMetaObject)
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
		bool isSupported = QOpcUaTypesConverter::isSupportedQType(returnType);
		Q_ASSERT_X(isSupported, "QOpcUaServer::addMetaMethods", "Return type not supported in MetaMethod.");
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
			outputArgumentInstance.description = UA_LOCALIZEDTEXT((char *)"en-US",
														  (char *)"Result Value");
			outputArgumentInstance.name        = QOpcUaTypesConverter::uaStringFromQString((char *)"Result");
			outputArgumentInstance.dataType    = QOpcUaTypesConverter::uaTypeNodeIdFromQType(returnType);
			outputArgumentInstance.valueRank   = UA_VALUERANK_SCALAR;
			outputArgument = &outputArgumentInstance;
		}
		// validate argument types and create them
		Q_ASSERT_X(metamethod.parameterCount() <= 10, "QOpcUaServer::addMetaMethods", "No more than 10 arguments supported in MetaMethod.");
		if (metamethod.parameterCount() > 10)
		{
			continue;
		}
		QVector<UA_Argument> vectArgs;
		auto listArgNames = metamethod.parameterNames();
		Q_ASSERT(listArgNames.count() == metamethod.parameterCount());
		for (int k = 0; k < metamethod.parameterCount(); k++)
		{
			isSupported = QOpcUaTypesConverter::isSupportedQType((QMetaType::Type)metamethod.parameterType(k));
			Q_ASSERT_X(isSupported, "QOpcUaServer::addMetaMethods", "Argument type not supported in MetaMethod.");
			if (!isSupported)
			{
				break;
			}
			UA_Argument inputArgument;
			UA_Argument_init(&inputArgument);
			// create n-th argument
			inputArgument.description = UA_LOCALIZEDTEXT((char *)"en-US", (char *)"Method Argument");
			inputArgument.name        = QOpcUaTypesConverter::uaStringFromQString(listArgNames.at(k));
			inputArgument.dataType    = QOpcUaTypesConverter::uaTypeNodeIdFromQType((QMetaType::Type)metamethod.parameterType(k));
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
		methAttr.description    = UA_LOCALIZEDTEXT((char *)"en-US",
												   strMethName.data());
		methAttr.displayName    = UA_LOCALIZEDTEXT((char *)"en-US",
												   strMethName.data());
		// create callback
		UA_NodeId methNodeId;
		auto st = UA_Server_addMethodNode(this->m_server,
										  UA_NODEID_NULL,
										  parentTypeNodeId,
										  UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
										  UA_QUALIFIEDNAME (1, strMethName.data()),
										  methAttr,
										  &QOpcUaServer::methodCallback,
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
		Q_ASSERT_X(!m_hashMethods.contains(methNodeId), "QOpcUaServer::addMetaMethods", "Method already exists, callback will be overwritten.");
		m_hashMethods[methNodeId] = [metamethod](void * objectContext, const UA_Variant * input, UA_Variant * output) {
			// get object instance that owns method
			QOpcUaBaseObject * object = static_cast<QOpcUaBaseObject*>(objectContext);
			Q_CHECK_PTR(object);
			if (!object)
			{
				return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
			}
			// convert input arguments to QVariants
			QVariantList varListArgs;
			QList<QGenericArgument> genListArgs;
			for (int k = 0; k < metamethod.parameterCount(); k++)
			{
				varListArgs.append(QOpcUaTypesConverter::uaVariantToQVariant(input[k]));
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
				*output = QOpcUaTypesConverter::uaVariantFromQVariant(returnValue);
			}
			// return success status
			return (UA_StatusCode)UA_STATUSCODE_GOOD;
		};
	}
}

UA_NodeId QOpcUaServer::createInstance(const QMetaObject & metaObject, QOpcUaServerNode * parentNode)
{
	// check if OPC UA relevant
	if (!metaObject.inherits(&QOpcUaServerNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QOpcUaServer::createInstance", "Unsupported base class");
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
	UA_NodeId referenceTypeId = QOpcUaServer::getReferenceTypeId(parentNode->metaObject()->className(), 
		                                                         metaObject.className());
	// set qualified name, default is class name
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name           = QOpcUaTypesConverter::uaStringFromQString(metaObject.className());
	// check if variable or object
	// NOTE : a type is considered to inherit itself (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
	UA_NodeId nodeIdNewInstance;
	if (metaObject.inherits(&QOpcUaBaseVariable::staticMetaObject))
	{
		UA_VariableAttributes vAttr = UA_VariableAttributes_default;
		// add variable
		auto st = UA_Server_addVariableNode(m_server,
                                            UA_NODEID_NULL,       // requested nodeId
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
		Q_ASSERT(metaObject.inherits(&QOpcUaBaseObject::staticMetaObject));
		UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		// add object
		auto st = UA_Server_addObjectNode(m_server,
                                          UA_NODEID_NULL,       // requested nodeId
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

void QOpcUaServer::bindCppInstanceWithUaNode(QOpcUaServerNode * nodeInstance, UA_NodeId & nodeId)
{
	Q_CHECK_PTR(nodeInstance);
	Q_ASSERT(!UA_NodeId_isNull(&nodeId));
	// set c++ instance as context
	UA_Server_setNodeContext(m_server, nodeId, (void**)(&nodeInstance));
	// set node id to c++ instance
	nodeInstance->m_nodeId = nodeId;
}

QOpcUaFolderObject * QOpcUaServer::get_objectsFolder()
{
	return m_pobjectsFolder;
}

UA_NodeId QOpcUaServer::getReferenceTypeId(const QString & strParentClassName, const QString & strChildClassName)
{
	UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	// adapt parent relation with child according to parent type
	if (strParentClassName.compare(QOpcUaFolderObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
	{
		referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	}
	else if (strParentClassName.compare(QOpcUaBaseObject      ::staticMetaObject.className(), Qt::CaseInsensitive) == 0 ||
		     strParentClassName.compare(QOpcUaBaseDataVariable::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
	{
		if (strChildClassName.compare(QOpcUaFolderObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0 ||
			strChildClassName.compare(QOpcUaBaseObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT);
		}
		else if (strChildClassName.compare(QOpcUaBaseDataVariable::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
		}
		else if (strChildClassName.compare(QOpcUaProperty::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
		}
	}
	else
	{
		Q_ASSERT_X(false, "QOpcUaServer::getReferenceTypeId", "Invalid parent type.");
	}
	
	return referenceTypeId;
}


