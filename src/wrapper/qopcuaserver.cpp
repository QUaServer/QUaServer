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
	Q_UNUSED(sessionId); 
	Q_UNUSED(sessionContext); 
	Q_UNUSED(typeNodeId); 
	Q_UNUSED(nodeContext);
	// get server from context object
	auto obj = static_cast<QOpcUaServer*>(typeNodeContext);
	Q_CHECK_PTR(obj);
	Q_ASSERT(obj->m_hashConstructors.contains(*typeNodeId));
	// get method from type constructors map and call it
	return obj->m_hashConstructors[*typeNodeId](server,
		                                        nodeId);
}

UA_StatusCode QOpcUaServer::uaConstructor(UA_Server         * server, 
	                                      const UA_NodeId   * nodeId, 
	                                      const QMetaObject & metaObject)
{
	// check if constructor from explicit instance creation or type registration
	UA_NodeId parentNodeId = QOpcUaServer::getParentNodeId(*nodeId, server);
	if (UA_NodeId_isNull(&parentNodeId))
	{
		// ignore if new instance is due to new type registration
		return UA_STATUSCODE_GOOD;
	}
	// is parent bound
	bool      parentBound = QOpcUaServer::isNodeBound(parentNodeId, server);
	QString   strClassName = QString(metaObject.className());
	qDebug() << "Called OPC UA constructor for" << strClassName << "Parent bound :" << parentBound;
	return UA_STATUSCODE_GOOD;
}

UA_NodeId QOpcUaServer::getParentNodeId(const UA_NodeId &childNodeId, UA_Server *server)
{
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	bDesc->nodeId          = childNodeId; // from child
	bDesc->browseDirection = UA_BROWSEDIRECTION_INVERSE; //  look upwards
	bDesc->includeSubtypes = false;
	bDesc->nodeClassMask   = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE; // only objects or variables (no types or refs)
	bDesc->resultMask      = UA_BROWSERESULTMASK_BROWSENAME | UA_BROWSERESULTMASK_DISPLAYNAME; // bring only useful info | UA_BROWSERESULTMASK_ALL;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(server, 0, bDesc);
	assert(bRes.statusCode == UA_STATUSCODE_GOOD);
	QList<UA_NodeId> listParents;
	while (bRes.referencesSize > 0)
	{
		for (size_t i = 0; i < bRes.referencesSize; i++)
		{
			UA_ReferenceDescription rDesc = bRes.references[i];
			UA_NodeId nodeId = rDesc.nodeId.nodeId;
			listParents.append(nodeId);
		}
		bRes = UA_Server_browseNext(server, true, &bRes.continuationPoint);
	}
	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);
	// return
	Q_ASSERT_X(listParents.count() <= 1, "QOpcUaServer::getParentNodeId", "Child code it not supposed to have more than one parent.");
	return listParents.count() > 0 ? listParents.at(0) : UA_NODEID_NULL;
}

bool QOpcUaServer::isNodeBound(const UA_NodeId & nodeId, UA_Server *server)
{
	// gte void context
	void * context;
	auto st = UA_Server_getNodeContext(server, nodeId, &context);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	if (st != UA_STATUSCODE_GOOD)
	{
		return false;
	}
	// try to cast to C++ node
	auto ptr = static_cast<QOpcUaServerNode*>(context);
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
	m_pobjectsFolder = new QOpcUaFolderObject(this);
	// [TEMP] bind objects folder
	auto st = UA_Server_setNodeContext(m_server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), (void*)m_pobjectsFolder);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	m_pobjectsFolder->m_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

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

void QOpcUaServer::registerTypeLifeCycle(const UA_NodeId * nodeId, const QMetaObject & metaObject)
{
	Q_CHECK_PTR(nodeId);
	Q_ASSERT(!UA_NodeId_isNull(nodeId));
	if (UA_NodeId_isNull(nodeId))
	{
		return;
	}
	// add constructor
	Q_ASSERT_X(!m_hashConstructors.contains(*nodeId), "QOpcUaServer::registerType", "Constructor for type already exists.");
	// NOTE : we need constructors to be lambdas in order to cachec metaobject in capture
	//        because type context is already the server instance where type was registered
	//        so we can differentiate the server instance in the static ::uaConstructor callback
	//        TLDR; to support multiple server instance in an application
	m_hashConstructors[*nodeId] = [metaObject](UA_Server        *server,
		                                       const UA_NodeId  *nodeId) {
		// call static method
		return QOpcUaServer::uaConstructor(server, nodeId, metaObject);
	};

	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &QOpcUaServer::uaConstructor;
	
	// TODO : destructor
	//lifecycle.destructor  = ;
	
	UA_Server_setNodeTypeLifecycle(m_server, *nodeId, lifecycle);
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
                                            nullptr,             // new instance as context (set LATER)
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
                                          nullptr,             // new instance as context (set LATER)
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


