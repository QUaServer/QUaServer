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
	UA_NodeId parentNodeId = QOpcUaServerNode::getParentNodeId(*nodeId, server->m_server);
	// ignore if new instance is due to new type registration
	if (UA_NodeId_isNull(&parentNodeId))
	{		
		return UA_STATUSCODE_GOOD;
	}
	// check if constructor from explicit instance creation or type registration
	// this is done by checking that eventually going up we reach the objects folder
	UA_NodeId objsFolderNodeId   = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	UA_NodeId parentParentNodeId = parentNodeId;
	while (!UA_NodeId_equal(&parentParentNodeId, &objsFolderNodeId))
	{
		parentParentNodeId = QOpcUaServerNode::getParentNodeId(parentParentNodeId, server->m_server);
		// ignore if new instance is due to new type registration
		if (UA_NodeId_isNull(&parentParentNodeId))
		{
			
			return UA_STATUSCODE_GOOD;
		}
	}
	qDebug() << metaObject.className();
	// create new instance (and bind it to UA, in base types happens in constructor, in derived class is done by QOpcUaServerNodeFactory)
	Q_ASSERT_X(metaObject.constructorCount() > 0, "QOpcUaServer::uaConstructor", "Failed instantiation. No matching Q_INVOKABLE constructor with signature CONSTRUCTOR(QOpcUaServer *server, const UA_NodeId &nodeId) found.");
	auto * pQObject    = metaObject.newInstance(Q_ARG(QOpcUaServer*, server), Q_ARG(UA_NodeId, *nodeId));
	Q_ASSERT_X(pQObject, "QOpcUaServer::uaConstructor", "Failed instantiation. No matching Q_INVOKABLE constructor with signature CONSTRUCTOR(QOpcUaServer *server, const UA_NodeId &nodeId) found.");
	auto * newInstance = static_cast<QOpcUaServerNode*>(pQObject);
	Q_CHECK_PTR(newInstance);
	// need to bind again using the official (void ** nodeContext) of the UA constructor
	// because we set context on C++ instantiation, but later the UA library overwrites it 
	// after calling the UA constructor
	*nodeContext = (void*)newInstance;
	newInstance->m_nodeId = *nodeId;
	// if parent bound, set as parent 
	auto parent = QOpcUaServerNode::getNodeContext(parentNodeId, server->m_server);
	if (parent)
	{
		Q_ASSERT(QOpcUaServer::isNodeBound(parentNodeId, server->m_server));
		newInstance->setParent(parent);
		// TODO : emit event in parent, that new children added
	}
	// if parent not bound it means parent is a non-base type and its c++ instance has not been created.
	// then the parent and server will be set later in QOpcUaServerNodeFactory when parent is instantiated.
	// the parent in QOpcUaServerNodeFactory will assign itself as parent if its children.

	// children should be assigned already to new instance, if the user inherited from QOpcUaServerNodeFactory<> appropriately
	// here we just verify
	auto chidrenNodeIds = QOpcUaServerNode::getChildrenNodeIds(*nodeId, server);
	int propCount  = metaObject.propertyCount();
	int propOffset = QOpcUaServerNode::getPropsOffsetHelper(metaObject);
	int numProps   = 0;
	for (int i = propOffset; i < propCount; i++)
	{
		// check if available in meta-system
		QMetaProperty metaproperty = metaObject.property(i);
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
		if (propMetaObject.inherits(&metaObject))
		{
			continue;
		}
		// inc number of valid props
		numProps++;
	}
	bool bChildrenBound = (chidrenNodeIds.count() == numProps);
	Q_ASSERT_X(bChildrenBound, "QOpcUaServer::uaConstructor", "Children not bound properly. Did you inherit from QOpcUaServerNodeFactory appropriately?");
	if (!bChildrenBound)
	{
		// failed to bind children before constructor, at least bind after constructor
		// TODO : reuse code from QOpcUaServerNodeFactory to bind children after constructor
	}

	// success
	return UA_STATUSCODE_GOOD;
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
	UA_ServerConfig *config  = UA_ServerConfig_new_default();
	this->m_server = UA_Server_new(config);
	m_running = false;
	// Create "Objects" folder using special constructor
	// Part 5 - 8.2.4 : Objects
	m_pobjectsFolder = new QOpcUaFolderObject(this, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
	m_pobjectsFolder->setParent(this);
	// register base types
	m_mapTypes.insert(QString(QOpcUaBaseVariable    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE)    );
	m_mapTypes.insert(QString(QOpcUaBaseDataVariable::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE));
	m_mapTypes.insert(QString(QOpcUaProperty        ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        );
	m_mapTypes.insert(QString(QOpcUaBaseObject      ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      );
	m_mapTypes.insert(QString(QOpcUaFolderObject    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          );
	// set context for base types
	UA_StatusCode st;
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , (void*)this); Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// register constructors for instantiable types
	this->registerTypeLifeCycle(&UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), QOpcUaBaseDataVariable::staticMetaObject);
	this->registerTypeLifeCycle(&UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        , QOpcUaProperty        ::staticMetaObject);
	this->registerTypeLifeCycle(&UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      , QOpcUaBaseObject      ::staticMetaObject);
	this->registerTypeLifeCycle(&UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          , QOpcUaFolderObject    ::staticMetaObject);
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

	// TODO : register meta-methods

	//


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
												nullptr,          // context : null because we want to filter it when calling ua constructor
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
											  nullptr,          // context : null because we want to filter it when calling ua constructor
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
                                            nullptr,             // new instance as context (set when c++ instance created)
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
                                          nullptr,             // new instance as context (set when c++ instance created)
                                          &nodeIdNewInstance); // set new nodeId to new instance
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
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


