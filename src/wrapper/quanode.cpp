#include "quanode.h"

#include <QUaServer>
#include <QUaProperty>
#include <QUaBaseDataVariable>
#include <QUaFolderObject>

QUaNode::QUaNode(QUaServer *server)
{
	// [NOTE] : constructor of any QUaNode-derived class is not meant to be called by the user
	//          the constructor is called automagically by this library, and m_newNodeNodeId and
	//          m_newNodeMetaObject must be set in QUaServer before calling the constructor, as
	//          is used in QUaServer::uaConstructor
	Q_CHECK_PTR(server);
	Q_CHECK_PTR(server->m_newNodeNodeId);
	Q_CHECK_PTR(server->m_newNodeMetaObject);
	const UA_NodeId   &nodeId     = *server->m_newNodeNodeId;
	const QMetaObject &metaObject = *server->m_newNodeMetaObject;
	// check
	Q_ASSERT(server && !UA_NodeId_isNull(&nodeId));
	// bind itself, only good for constructors of derived classes, because UA constructor overwrites it
	// so we need to also set the context again in QUaServer::uaConstructor
	// set server instance
	this->m_qUaServer = server;
	// set c++ instance as context
	UA_Server_setNodeContext(server->m_server, nodeId, (void*)this);
	// set node id to c++ instance
	this->m_nodeId = nodeId;
	// ignore objects folder
	UA_NodeId objectsFolderNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	if (UA_NodeId_equal(&nodeId, &objectsFolderNodeId))
	{
		return;
	}
	// get all UA children in advance, because if none, then better early exit
	auto chidrenNodeIds = QUaNode::getChildrenNodeIds(nodeId, server);
	if (chidrenNodeIds.count() <= 0)
	{
		return;
	}
	// create hash of nodeId's by browse name, which must match Qt's metaprops
	QHash<QString, UA_NodeId> mapChildren;
	for (int i = 0; i < chidrenNodeIds.count(); i++)
	{
		auto childNodeId = chidrenNodeIds[i];
		// read browse name
		QString strBrowseName = QUaNode::getBrowseName(childNodeId, server);
		Q_ASSERT(!mapChildren.contains(strBrowseName));
		mapChildren[strBrowseName] = childNodeId;
	}
	// list meta props
	int propCount  = metaObject.propertyCount();
	int propOffset = QUaNode::getPropsOffsetHelper(metaObject);
	int numProps   = 0;
	for (int i = propOffset; i < propCount; i++)
	{
		QMetaProperty metaProperty = metaObject.property(i);
		// check if not enum
		if (!metaProperty.isEnumType())
		{
			// check if available in meta-system
			if (!QMetaType::metaObjectForType(metaProperty.userType()))
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
			Q_ASSERT_X(!propMetaObject.inherits(&metaObject), "QOpcUaServerNodeFactory", "Qt MetaProperty type cannot inherit from Class.");
			if (propMetaObject.inherits(&metaObject))
			{
				continue;
			}
		}
		// inc number of valid props
		numProps++;
		// the Qt meta property name must match the UA browse name
		QString strBrowseName = QString(metaProperty.name());
		Q_ASSERT(mapChildren.contains(strBrowseName));
		// get child nodeId for child
		auto childNodeId = mapChildren.take(strBrowseName);
		// get node context (C++ instance)
		auto nodeInstance = QUaNode::getNodeContext(childNodeId, server);
		Q_CHECK_PTR(nodeInstance);
		// assign C++ parent
		nodeInstance->setParent(this);
		nodeInstance->setObjectName(strBrowseName);
		// [NOTE] writing a pointer value to a Q_PROPERTY did not work, 
		//        eventhough there appear to be some success cases on the internet
		//        so in the end we have to query children by object name
	} // for props
	// handle events
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// do not assert if event type
	if (metaObject.inherits(&QUaBaseEvent::staticMetaObject))
	{
		for (int i = 0; i < QUaBaseEvent::listDefaultProps.count(); i++)
		{
			QString strBrowseName = QUaBaseEvent::listDefaultProps.at(i);
			Q_ASSERT(mapChildren.contains(strBrowseName));
			// get child nodeId for child
			auto childNodeId = mapChildren.take(strBrowseName);
			// get node context (C++ instance)
			auto nodeInstance = QUaNode::getNodeContext(childNodeId, server);
			Q_CHECK_PTR(nodeInstance);
			// assign C++ parent
			nodeInstance->setParent(this);
			nodeInstance->setObjectName(strBrowseName);
		}
		Q_ASSERT_X(mapChildren.count() == 0, "QUaNode::QUaNode", "Event children not bound properly.");
		return;
	}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// if assert below fails, review filter in QUaNode::getChildrenNodeIds
	Q_ASSERT_X(mapChildren.count()      == 0        &&
		       chidrenNodeIds.count()   == numProps &&
		       this->children().count() == numProps, "QUaNode::QUaNode", "Children not bound properly.");
}

QUaNode::~QUaNode()
{
	// check if node id has been already removed from node store
	// i.e. child of deleted parent node, or ...
	UA_NodeId outNodeId;
	auto st = UA_Server_readNodeId(m_qUaServer->m_server, m_nodeId, &outNodeId);
	if (st == UA_STATUSCODE_BADNODEIDUNKNOWN)
	{
		return;
	}
	Q_ASSERT(UA_NodeId_equal(&m_nodeId, &outNodeId));
	// remove context, so we avoid double deleting in ua destructor when called
	st = UA_Server_setNodeContext(m_qUaServer->m_server, m_nodeId, nullptr);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// delete node in ua (NOTE : also delete references)
	st = UA_Server_deleteNode(m_qUaServer->m_server, m_nodeId, true);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
}

bool QUaNode::operator==(const QUaNode & other) const
{
	return UA_NodeId_equal(&this->m_nodeId, &other.m_nodeId);
}

QString QUaNode::displayName() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read description
	UA_LocalizedText outDisplayName;
	auto st = UA_Server_readDisplayName(m_qUaServer->m_server, m_nodeId, &outDisplayName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return
	return QUaTypesConverter::uaStringToQString(outDisplayName.text);
}

void QUaNode::setDisplayName(const QString & displayName)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDisplayName = displayName.toUtf8(); // NOTE : QByteArray must exist in stack
    UA_LocalizedText uaDisplayName = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
	// set value
	auto st = UA_Server_writeDisplayName(m_qUaServer->m_server, m_nodeId, uaDisplayName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit displayName changed
	emit this->displayNameChanged(displayName);
}

QString QUaNode::description() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read description
	UA_LocalizedText outDescription;
	auto st = UA_Server_readDescription(m_qUaServer->m_server, m_nodeId, &outDescription);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return
	return QUaTypesConverter::uaStringToQString(outDescription.text);
}

void QUaNode::setDescription(const QString & description)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDescription = description.toUtf8(); // NOTE : QByteArray must exist in stack
	UA_LocalizedText uaDescription = UA_LOCALIZEDTEXT((char*)"", byteDescription.data());
	// set value
	auto st = UA_Server_writeDescription(m_qUaServer->m_server, m_nodeId, uaDescription);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit description changed
	emit this->descriptionChanged(description);
}

quint32 QUaNode::writeMask() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return quint32();
	}
	// read writeMask
	UA_UInt32 outWriteMask;
	auto st = UA_Server_readWriteMask(m_qUaServer->m_server, m_nodeId, &outWriteMask);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return
	return outWriteMask;
}

void QUaNode::setWriteMask(const quint32 & writeMask)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set value
	auto st = UA_Server_writeWriteMask(m_qUaServer->m_server, m_nodeId, writeMask);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit writeMask changed
	emit this->writeMaskChanged(writeMask);
}

QString QUaNode::nodeId() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	return QUaTypesConverter::nodeIdToQString(m_nodeId);
}

QString QUaNode::nodeClass() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read nodeClass
	UA_NodeClass outNodeClass;
	auto st = UA_Server_readNodeClass(m_qUaServer->m_server, m_nodeId, &outNodeClass);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// convert to QString
	return QUaTypesConverter::nodeClassToQString(outNodeClass);
}

QString QUaNode::browseName() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read browse name
	UA_QualifiedName outBrowseName;
	auto st = UA_Server_readBrowseName(m_qUaServer->m_server, m_nodeId, &outBrowseName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// populate return value
	// NOTE : ignore Namespace index outBrowseName.namespaceIndex
	return QUaTypesConverter::uaStringToQString(outBrowseName.name);
}

void QUaNode::setBrowseName(const QString & browseName)
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_QualifiedName
	UA_QualifiedName bName;
	bName.namespaceIndex = 1; // NOTE : force default namespace index 1
	bName.name           = QUaTypesConverter::uaStringFromQString(browseName);
	// set value
	auto st = UA_Server_writeBrowseName(m_qUaServer->m_server, m_nodeId, bName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// also update QObject name
	this->setObjectName(browseName);
	// emit browseName changed
	emit this->browseNameChanged(browseName);
}

QUaProperty * QUaNode::addProperty(const QString &strNodeId/* = ""*/)
{
	return m_qUaServer->createInstance<QUaProperty>(this, strNodeId);
}

QUaBaseDataVariable * QUaNode::addBaseDataVariable(const QString &strNodeId/* = ""*/)
{
	return m_qUaServer->createInstance<QUaBaseDataVariable>(this, strNodeId);
}

QUaBaseObject * QUaNode::addBaseObject(const QString &strNodeId/* = ""*/)
{
	return m_qUaServer->createInstance<QUaBaseObject>(this, strNodeId);
}

QUaFolderObject * QUaNode::addFolderObject(const QString &strNodeId/* = ""*/)
{
	return m_qUaServer->createInstance<QUaFolderObject>(this, strNodeId);
}

QList<QUaNode*> QUaNode::browseChildren(const QString &strBrowseName/* = QString()*/)
{
	return this->findChildren<QUaNode*>(strBrowseName, Qt::FindDirectChildrenOnly);
}

QUaNode* QUaNode::browseChild(const QString & strBrowseName)
{
	auto children = this->browseChildren(strBrowseName);
	if (children.isEmpty())
	{
		return nullptr;
	}
	return children.first();
}

QUaNode * QUaNode::browsePath(const QStringList & strBrowsePath)
{
	QUaNode * currNode = this;
	for (int i = 0; i < strBrowsePath.count(); i++)
	{
		currNode = currNode->browseChild(strBrowsePath.at(i));
		if (!currNode)
		{
			return nullptr;
		}
	}
	return currNode;
}

bool QUaNode::hasChild(const QString & strBrowseName)
{
	return !this->browseChildren(strBrowseName).isEmpty();
}

void QUaNode::addReference(const QUaReference & ref, const QUaNode * nodeTarget, const bool & isForward/* = true*/)
{
	// first check if reference type is registered
	if (!m_qUaServer->m_hashRefs.contains(ref))
	{
		m_qUaServer->registerReference(ref);
	}
	Q_ASSERT(m_qUaServer->m_hashRefs.contains(ref));
	// check valid node target
	Q_ASSERT_X(nodeTarget, "QUaNode::addReference", "Invalid target node.");
	if (!nodeTarget)
	{
		return;
	}
	UA_NodeId refTypeId = m_qUaServer->m_hashRefs[ref];
	// check if reference already exists
	auto set = getRefsInternal(ref, isForward);
	if (set.contains(nodeTarget->m_nodeId))
	{
		return;
	}
	// add the reference
	auto st = UA_Server_addReference(
		m_qUaServer->m_server,
		m_nodeId,
		refTypeId,
		{ nodeTarget->m_nodeId, UA_STRING_NULL, 0 },
		isForward
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
}

void QUaNode::removeReference(const QUaReference & ref, const QUaNode * nodeTarget, const bool & isForward/* = true*/)
{
	// first check if reference type is removeReference
	Q_ASSERT_X(m_qUaServer->m_hashRefs.contains(ref), "QUaNode::addReference", "Reference not registered.");
	if (!m_qUaServer->m_hashRefs.contains(ref))
	{
		return;
	}
	// check valid node target
	Q_ASSERT_X(nodeTarget, "QUaNode::removeReference", "Invalid target node.");
	if (!nodeTarget)
	{
		return;
	}
	UA_NodeId refTypeId = m_qUaServer->m_hashRefs[ref];
	// check reference exists
	auto set = getRefsInternal(ref, isForward);
	if (!set.contains(nodeTarget->m_nodeId))
	{
		return;
	}
	// remove the reference
	auto st = UA_Server_deleteReference(
		m_qUaServer->m_server,
		m_nodeId,
		refTypeId,
		isForward,
		{ nodeTarget->m_nodeId, UA_STRING_NULL, 0 },
		true
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
}

QList<QUaNode*> QUaNode::findReferences(const QUaReference & ref, const bool & isForward)
{
	QList<QUaNode*> retRefList;
	// call internal method
	auto set = getRefsInternal(ref, isForward);
	// get contexts
	QSetIterator<UA_NodeId> i(set);
	while (i.hasNext())
	{
		QUaNode * node = QUaNode::getNodeContext(i.next(), m_qUaServer);
		Q_CHECK_PTR(node);
		if (node)
		{
			retRefList.append(node);
		}
	}	
	// return
	return retRefList;
}

QSet<UA_NodeId> QUaNode::getRefsInternal(const QUaReference & ref, const bool & isForward)
{
	QSet<UA_NodeId> retRefSet;
	// first check if reference type is registered
	if (!m_qUaServer->m_hashRefs.contains(ref))
	{
		m_qUaServer->registerReference(ref);
		// there cannot be any since it didnt even exist before
		return retRefSet;
	}
	Q_ASSERT(m_qUaServer->m_hashRefs.contains(ref));
	UA_NodeId refTypeId = m_qUaServer->m_hashRefs[ref];
	// make ua browse
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	UA_NodeId_copy(&m_nodeId, &bDesc->nodeId); // from child
	bDesc->browseDirection = isForward ? UA_BROWSEDIRECTION_FORWARD : UA_BROWSEDIRECTION_INVERSE;
	bDesc->includeSubtypes = true;
	bDesc->resultMask      = UA_BROWSERESULTMASK_REFERENCETYPEID;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(m_qUaServer->m_server, 0, bDesc);
	Q_ASSERT(bRes.statusCode == UA_STATUSCODE_GOOD);
	while (bRes.referencesSize > 0)
	{
		for (size_t i = 0; i < bRes.referencesSize; i++)
		{
			UA_ReferenceDescription rDesc = bRes.references[i];
			if (UA_NodeId_equal(&rDesc.referenceTypeId, &refTypeId))
			{
				UA_NodeId nodeId = rDesc.nodeId.nodeId;
				retRefSet.insert(nodeId);
			}
		}
		bRes = UA_Server_browseNext(m_qUaServer->m_server, true, &bRes.continuationPoint);
	}
	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);
	// return
	return retRefSet;
}

QUaWriteMask QUaNode::userWriteMaskInternal(const QString & strUserName)
{
	// if has specific callback, use that one
	if (m_userWriteMaskCallback)
	{
		return m_userWriteMaskCallback(strUserName);
	}
	// else use possible reimplementation
	return this->userWriteMask(strUserName);
}

QUaAccessLevel QUaNode::userAccessLevelInternal(const QString & strUserName)
{
	// if has specific callback, use that one
	if (m_userAccessLevelCallback)
	{
		return m_userAccessLevelCallback(strUserName);
	}
	// else use possible reimplementation
	return this->userAccessLevel(strUserName);
}

bool QUaNode::userExecutableInternal(const QString & strUserName)
{
	// if has specific callback, use that one
	if (m_userExecutableCallback)
	{
		return m_userExecutableCallback(strUserName);
	}
	// else use possible reimplementation
	return this->userExecutable(strUserName);
}

QUaWriteMask QUaNode::userWriteMask(const QString & strUserName)
{
	// if has a node parent, use parent's callback
	QUaNode * parent = dynamic_cast<QUaNode*>(this->parent());
	if (parent)
	{
		return parent->userWriteMask(strUserName);
	}
	// else all permissions
	return 0xFFFFFFFF;
}

QUaAccessLevel QUaNode::userAccessLevel(const QString & strUserName)
{
	// if has a node parent, use parent's callback
	QUaNode * parent = dynamic_cast<QUaNode*>(this->parent());
	if (parent)
	{
		return parent->userAccessLevelInternal(strUserName);
	}
	// else all permissions
	return 0xFF;
}

bool QUaNode::userExecutable(const QString & strUserName)
{
	// if has a node parent, use parent's callback
	QUaNode * parent = dynamic_cast<QUaNode*>(this->parent());
	if (parent)
	{
		return parent->userExecutable(strUserName);
	}
	// else allow
	return true;
}

UA_NodeId QUaNode::getParentNodeId(const UA_NodeId & childNodeId, QUaServer * server)
{
	return QUaNode::getParentNodeId(childNodeId, server->m_server);
}

UA_NodeId QUaNode::getParentNodeId(const UA_NodeId & childNodeId, UA_Server * server)
{
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	UA_NodeId_copy(&childNodeId, &bDesc->nodeId); // from child
	bDesc->browseDirection = UA_BROWSEDIRECTION_INVERSE; //  look upwards
	bDesc->includeSubtypes = true;
	bDesc->resultMask      = UA_BROWSERESULTMASK_BROWSENAME | UA_BROWSERESULTMASK_DISPLAYNAME;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(server, 0, bDesc);
	Q_ASSERT(bRes.statusCode == UA_STATUSCODE_GOOD);
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
	// check if method
	UA_NodeClass outNodeClass;
	UA_Server_readNodeClass(server, childNodeId, &outNodeClass);
	// NOTE : seems methods added to subtype have references to all instances created of the subtype
	// TODO : fix, when https://github.com/open62541/open62541/pull/1812 is fixed
	Q_ASSERT_X(
		listParents.count() <= 1 && outNodeClass != UA_NODECLASS_METHOD ||
		listParents.count() >= 1 && outNodeClass == UA_NODECLASS_METHOD,
		"QUaServer::getParentNodeId", "Child code it not supposed to have more than one parent.");
	// return
	return listParents.count() > 0 ? listParents.at(0) : UA_NODEID_NULL;
}

QList<UA_NodeId> QUaNode::getChildrenNodeIds(const UA_NodeId & parentNodeId, QUaServer * server)
{
	return QUaNode::getChildrenNodeIds(parentNodeId, server->m_server);
}

QList<UA_NodeId> QUaNode::getChildrenNodeIds(const UA_NodeId & parentNodeId, UA_Server * server)
{
	QList<UA_NodeId> retListChildren;
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	UA_NodeId_copy(&parentNodeId, &bDesc->nodeId); // from parent
	bDesc->browseDirection = UA_BROWSEDIRECTION_FORWARD; //  look downwards
	bDesc->includeSubtypes = false;
	bDesc->nodeClassMask   = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE; // only objects or variables (no types or refs)
	bDesc->resultMask      = UA_BROWSERESULTMASK_BROWSENAME | UA_BROWSERESULTMASK_DISPLAYNAME; // bring only useful info | UA_BROWSERESULTMASK_ALL;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(server, 0, bDesc);
	assert(bRes.statusCode == UA_STATUSCODE_GOOD);
	while (bRes.referencesSize > 0)
	{
		for (size_t i = 0; i < bRes.referencesSize; i++)
		{
			UA_ReferenceDescription rDesc = bRes.references[i];
			UA_NodeId nodeId = rDesc.nodeId.nodeId;
			// ignore modelling rules
            auto nodeIdMandatory = UA_NODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY);
            if (UA_NodeId_equal(&nodeId, &nodeIdMandatory))
			{
				continue;
			}
			retListChildren.append(nodeId);
		}
		bRes = UA_Server_browseNext(server, true, &bRes.continuationPoint);
	}
	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);
	// return
	return retListChildren;
}

QUaNode * QUaNode::getNodeContext(const UA_NodeId & nodeId, QUaServer * server)
{
	return QUaNode::getNodeContext(nodeId, server->m_server);
}

QUaNode * QUaNode::getNodeContext(const UA_NodeId & nodeId, UA_Server * server)
{
	void * context = QUaNode::getVoidContext(nodeId, server);
	// try to cast to C++ node
	return dynamic_cast<QUaNode*>(static_cast<QObject*>(context));
}

void * QUaNode::getVoidContext(const UA_NodeId & nodeId, UA_Server * server)
{
	// get void context
	void * context;
	auto st = UA_Server_getNodeContext(server, nodeId, &context);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	if (st != UA_STATUSCODE_GOOD)
	{
		return nullptr;
	}
	// try to cast to C++ node
	return context;
}

QString QUaNode::getBrowseName(const UA_NodeId & nodeId, QUaServer * server)
{
	return QUaNode::getBrowseName(nodeId, server->m_server);
}

QString QUaNode::getBrowseName(const UA_NodeId & nodeId, UA_Server * server)
{
	// read browse name
	UA_QualifiedName outBrowseName;
	auto st = UA_Server_readBrowseName(server, nodeId, &outBrowseName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// NOTE : ignore Namespace index outBrowseName.namespaceIndex
	return QUaTypesConverter::uaStringToQString(outBrowseName.name);
}

int QUaNode::getPropsOffsetHelper(const QMetaObject & metaObject)
{
	int propOffset;
	// need to set props also inherited from base class
	if (metaObject.inherits(&QUaBaseVariable::staticMetaObject))
	{
		propOffset = QUaBaseVariable::staticMetaObject.propertyOffset();
	}
	else
	{
		// [NOTE] : assert below does not work, dont know why
		//Q_ASSERT(metaObject.inherits(&QUaBaseObject::staticMetaObject));
		propOffset = QUaBaseObject::staticMetaObject.propertyOffset();
	}
	return propOffset;
}
