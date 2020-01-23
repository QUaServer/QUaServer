#include "quanode.h"

#include <QUaServer>
#include <QUaProperty>
#include <QUaBaseDataVariable>
#include <QUaFolderObject>

// empty list of default variables for a node
const QStringList QUaNode::DefaultProperties = QStringList();

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
	// handle variables of known types defined in the standard (not custom types)
	auto listDefaultProps = server->m_newEventDefaultProperties;
	if (metaObject.inherits(&QUaBaseEvent::staticMetaObject))
	{
		for (int i = 0; i < listDefaultProps->count(); i++)
		{
			QString strBrowseName = listDefaultProps->at(i);
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
	// cleanup
	for (int i = 0; i < chidrenNodeIds.count(); i++)
	{
		UA_NodeId_clear(&chidrenNodeIds[i]);
	}
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
		// cleanup
		UA_NodeId_clear(&outNodeId);
		return;
	}
	Q_ASSERT(UA_NodeId_equal(&m_nodeId, &outNodeId));
	// cleanup
	UA_NodeId_clear(&outNodeId);
	// remove context, so we avoid double deleting in ua destructor when called
	st = UA_Server_setNodeContext(m_qUaServer->m_server, m_nodeId, nullptr);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	// delete node in ua (NOTE : also delete references)
	st = UA_Server_deleteNode(m_qUaServer->m_server, m_nodeId, true);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// trigger reference deleted, model change event, so client (UaExpert) auto refreshes tree
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	Q_CHECK_PTR(m_qUaServer->m_changeEvent);
	QUaNode* parent = qobject_cast<QUaNode*>(this->parent());
	if (!parent)
	{
		return;
	}
	// add reference deleted change to buffer
	m_qUaServer->addChange({
		parent->nodeId(),
		parent->typeDefinitionNodeId(),
		QUaChangeVerb::ReferenceDeleted // UaExpert does not recognize QUaChangeVerb::NodeAdded
	});	
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
}

bool QUaNode::operator==(const QUaNode & other) const
{
	return UA_NodeId_equal(&this->m_nodeId, &other.m_nodeId);
}

QUaServer * QUaNode::server() const
{
	return m_qUaServer;
}

QString QUaNode::displayName() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read display name
	UA_LocalizedText outDisplayName;
	auto st = UA_Server_readDisplayName(m_qUaServer->m_server, m_nodeId, &outDisplayName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	QString strDisplayName = QUaTypesConverter::uaStringToQString(outDisplayName.text);
	// cleanup
	UA_LocalizedText_clear(&outDisplayName);
	// return
	return strDisplayName;
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
	QString strBrowseName = QUaTypesConverter::uaStringToQString(outBrowseName.name);
	// cleanup
	UA_QualifiedName_clear(&outBrowseName);
	return strBrowseName;
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
	UA_QualifiedName_clear(&bName);
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

QString QUaNode::typeDefinitionNodeId() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	UA_NodeId retTypeId = UA_NODEID_NULL;
	// make ua browse
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	UA_NodeId_copy(&m_nodeId, &bDesc->nodeId); // from child
	bDesc->browseDirection = UA_BROWSEDIRECTION_FORWARD;
	bDesc->includeSubtypes = true;
	bDesc->resultMask      = UA_BROWSERESULTMASK_REFERENCETYPEID;
	bDesc->referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
	// browse
	UA_BrowseResult bRes = UA_Server_browse(m_qUaServer->m_server, 0, bDesc);
	Q_ASSERT(bRes.statusCode == UA_STATUSCODE_GOOD);
	Q_ASSERT(bRes.referencesSize == 1);
	if (bRes.referencesSize < 1)
	{
		return "";
	}
	UA_ReferenceDescription rDesc = bRes.references[0];
	UA_NodeId_copy(&rDesc.nodeId.nodeId, &retTypeId);
	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);
	// return in string form
	QString retNodeId = QUaTypesConverter::nodeIdToQString(retTypeId);
	UA_NodeId_clear(&retTypeId);
	return retNodeId;
}

QString QUaNode::typeDefinitionDisplayName() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	UA_NodeId typeId = QUaTypesConverter::nodeIdFromQString(this->typeDefinitionNodeId());
	Q_ASSERT(!UA_NodeId_isNull(&typeId));
	if (UA_NodeId_isNull(&typeId))
	{
		return QString();
	}
	// read display name
	UA_LocalizedText outDisplayName;
	auto st = UA_Server_readDisplayName(m_qUaServer->m_server, typeId, &outDisplayName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	QString strDisplayName = QUaTypesConverter::uaStringToQString(outDisplayName.text);
	// cleanup
	UA_NodeId_clear(&typeId);
	UA_LocalizedText_clear(&outDisplayName);
	// return
	return strDisplayName;
}

QString QUaNode::typeDefinitionBrowseName() const
{
	Q_CHECK_PTR(m_qUaServer);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	UA_NodeId typeId = QUaTypesConverter::nodeIdFromQString(this->typeDefinitionNodeId());
	Q_ASSERT(!UA_NodeId_isNull(&typeId));
	if (UA_NodeId_isNull(&typeId))
	{
		return QString();
	}
	// read browse name
	UA_QualifiedName outBrowseName;
	auto st = UA_Server_readBrowseName(m_qUaServer->m_server, typeId, &outBrowseName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// populate return value
	// NOTE : ignore Namespace index outBrowseName.namespaceIndex
	QString strBrowseName = QUaTypesConverter::uaStringToQString(outBrowseName.name);
	// cleanup
	UA_QualifiedName_clear(&outBrowseName);
	return strBrowseName;
}

QList<QUaNode*> QUaNode::browseChildren(const QString &strBrowseName/* = QString()*/) const
{
	return this->findChildren<QUaNode*>(strBrowseName, Qt::FindDirectChildrenOnly);
}

QUaNode* QUaNode::browseChild(const QString & strBrowseName) const
{
	auto children = this->browseChildren(strBrowseName);
	if (children.isEmpty())
	{
		return nullptr;
	}
	return children.first();
}

bool QUaNode::hasChild(const QString & strBrowseName)
{
	return !this->browseChildren(strBrowseName).isEmpty();
}

QUaNode * QUaNode::browsePath(const QStringList & strBrowsePath) const
{
	QUaNode * currNode = const_cast<QUaNode *>(this);
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

QStringList QUaNode::nodeBrowsePath() const
{
	// get parents browse path and then attach current browse name
	// stop recursion if current node is ObjectsFolder
	if (this == m_qUaServer->objectsFolder())
	{
		return QStringList() << this->browseName();
	}
#ifdef QT_DEBUG 
	QUaNode* parent = qobject_cast<QUaNode*>(this->parent());
	Q_CHECK_PTR(parent);
#else
	QUaNode* parent = static_cast<QUaNode*>(this->parent());
#endif // QT_DEBUG 
	return parent->nodeBrowsePath() << this->browseName();
}

void QUaNode::addReference(const QUaReferenceType& ref, QUaNode* nodeTarget, const bool& isForward/* = true*/)
{
	// first check if reference type is registered
	if (!m_qUaServer->m_hashRefTypes.contains(ref))
	{
		m_qUaServer->registerReferenceType(ref);
	}
	Q_ASSERT(m_qUaServer->m_hashRefTypes.contains(ref));
	// reject hierarchical references
	Q_ASSERT_X(!m_qUaServer->m_hashHierRefTypes.contains(ref), "QUaNode::addReference", "Cannot add hierarchical references using this method.");
	if (m_qUaServer->m_hashHierRefTypes.contains(ref))
	{
		return;
	}
	// check valid node target
	Q_ASSERT_X(nodeTarget, "QUaNode::addReference", "Invalid target node.");
	if (!nodeTarget)
	{
		return;
	}
	UA_NodeId refTypeId = m_qUaServer->m_hashRefTypes[ref];
	// check if reference already exists
	auto set = getRefsInternal(ref, isForward);
	if (set.contains(nodeTarget->m_nodeId))
	{
		// cleanup set
		QSetIterator<UA_NodeId> i(set);
		while (i.hasNext())
		{
			UA_NodeId nodeId = i.next();
			UA_NodeId_clear(&nodeId);
		}
		return;
	}
	// cleanup set
	QSetIterator<UA_NodeId> i(set);
	while (i.hasNext())
	{
		UA_NodeId nodeId = i.next();
		UA_NodeId_clear(&nodeId);
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
	// emit events
	emit this->referenceAdded(ref, nodeTarget, isForward);
	emit nodeTarget->referenceAdded(ref, this, !isForward);
	// subscribe node destructions
	QObject::connect(nodeTarget, &QObject::destroyed, this,
		[this, ref, nodeTarget, isForward]() {
			// emit event
			emit this->referenceRemoved(ref, nodeTarget, isForward);
		});
	QObject::connect(this, &QObject::destroyed, nodeTarget,
		[this, ref, nodeTarget, isForward]() {
			// emit event
			emit nodeTarget->referenceRemoved(ref, this, !isForward);
		});
}

void QUaNode::removeReference(const QUaReferenceType& ref, QUaNode* nodeTarget, const bool& isForward/* = true*/)
{
	// first check if reference type is removeReference
	Q_ASSERT_X(m_qUaServer->m_hashRefTypes.contains(ref), "QUaNode::addReference", "Reference not registered.");
	if (!m_qUaServer->m_hashRefTypes.contains(ref))
	{
		return;
	}
	// check valid node target
	Q_ASSERT_X(nodeTarget, "QUaNode::removeReference", "Invalid target node.");
	if (!nodeTarget)
	{
		return;
	}
	UA_NodeId refTypeId = m_qUaServer->m_hashRefTypes[ref];
	// check reference exists
	auto set = getRefsInternal(ref, isForward);
	if (!set.contains(nodeTarget->m_nodeId))
	{
		// cleanup set
		QSetIterator<UA_NodeId> i(set);
		while (i.hasNext())
		{
			UA_NodeId nodeId = i.next();
			UA_NodeId_clear(&nodeId);
		}
		return;
	}
	// cleanup set
	QSetIterator<UA_NodeId> i(set);
	while (i.hasNext())
	{
		UA_NodeId nodeId = i.next();
		UA_NodeId_clear(&nodeId);
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
	// emit event
	emit this->referenceRemoved(ref, nodeTarget, isForward);
	emit nodeTarget->referenceRemoved(ref, this, !isForward);
	// unsubscribe node destructions
	QObject::disconnect(nodeTarget, &QObject::destroyed, this, 0);
	QObject::disconnect(this, &QObject::destroyed, nodeTarget, 0);
}

QList<QUaNode*> QUaNode::findReferences(const QUaReferenceType& ref, const bool& isForward) const
{
	QList<QUaNode*> retRefList;
	// call internal method
	auto set = getRefsInternal(ref, isForward);
	// get contexts
	QSetIterator<UA_NodeId> i(set);
	while (i.hasNext())
	{
		UA_NodeId nodeId = i.next();
		QUaNode* node = QUaNode::getNodeContext(nodeId, m_qUaServer);
		if (node)
		{
			retRefList.append(node);
		}
		// cleanup set
		UA_NodeId_clear(&nodeId);
	}
	// return
	return retRefList;
}

// NOTE : need to cleanup result after calling this method
QSet<UA_NodeId> QUaNode::getRefsInternal(const QUaReferenceType& ref, const bool & isForward) const
{
	QSet<UA_NodeId> retRefSet;
	// first check if reference type is registered
	if (!m_qUaServer->m_hashRefTypes.contains(ref))
	{
		m_qUaServer->registerReferenceType(ref);
		// there cannot be any since it didnt even exist before
		return retRefSet;
	}
	Q_ASSERT(m_qUaServer->m_hashRefTypes.contains(ref));
	UA_NodeId refTypeId = m_qUaServer->m_hashRefTypes[ref];
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
				UA_NodeId nodeId/* = rDesc.nodeId.nodeId*/;
				UA_NodeId_copy(&rDesc.nodeId.nodeId, &nodeId);
				retRefSet.insert(nodeId);
			}
		}
		UA_BrowseResult_deleteMembers(&bRes);
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

const QMap<QString, QVariant> QUaNode::serializeAttrs() const
{
	QMap<QString, QVariant> retMap;
	auto metaObject = this->metaObject();
	// list meta props
	int propCount  = metaObject->propertyCount();
	int propOffset = QUaNode::staticMetaObject.propertyOffset();
	for (int i = propOffset; i < propCount; i++)
	{
		QMetaProperty metaProperty = metaObject->property(i);
		QString strPropName = QString(metaProperty.name());
		Q_ASSERT(!retMap.contains(strPropName));
		Q_ASSERT(metaProperty.isReadable());
		// non-writabe props cannot be restored
		if (!metaProperty.isWritable())
		{
			continue;
		}
		// ignore QUaNode * properties
		int type = metaProperty.userType();
		auto propMetaObject = QMetaType::metaObjectForType(type);
		if (propMetaObject && propMetaObject->inherits(&QUaNode::staticMetaObject))
		{
			continue;
		}
		QVariant value = metaProperty.read(this);
		// check if
		auto metaType = static_cast<QMetaType::Type>(type);
		if (metaType == QMetaType::UnknownType)
		{
			continue;
		}
		// get the Qt meta property name
		retMap[strPropName] = value;
	}
	return retMap;
}

const QList<QUaForwardReference> QUaNode::serializeRefs() const
{
	QList<QUaForwardReference> retList;
	for (auto& refType : m_qUaServer->referenceTypes())
	{
		for (auto ref : this->findReferences(refType))
		{
			QUaForwardReference fRef = { 
				ref->nodeId(), 
				ref->metaObject()->className(),
				refType 
			};
			retList << fRef;
		}
	}
	return retList;
}

void QUaNode::deserializeAttrs(const QMap<QString, QVariant>& attrs, QQueue<QUaLog>& logOut)
{
	// TODO : implement
}

QUaWriteMask QUaNode::userWriteMask(const QString & strUserName)
{
	// if has a node parent, use parent's callback
	QUaNode * parent = qobject_cast<QUaNode*>(this->parent());
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
	QUaNode * parent = qobject_cast<QUaNode*>(this->parent());
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
	QUaNode * parent = qobject_cast<QUaNode*>(this->parent());
	if (parent)
	{
		return parent->userExecutable(strUserName);
	}
	// else allow
	return true;
}

// NOTE : need to cleanup result after calling this method
UA_NodeId QUaNode::getParentNodeId(const UA_NodeId & childNodeId, QUaServer * server)
{
	return QUaNode::getParentNodeId(childNodeId, server->m_server);
}

// NOTE : need to cleanup result after calling this method
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
			// NOTE : it seems cleanup below also deletes the strings of string nodeIds which creates a bug
			//        this means we need to cleanup the result everytime we call QUaNode::getParentNodeId
			//UA_NodeId nodeId = rDesc.nodeId.nodeId;
			UA_NodeId nodeId;
			UA_NodeId_copy(&rDesc.nodeId.nodeId, &nodeId);
			listParents.append(nodeId);
		}
		UA_BrowseResult_deleteMembers(&bRes);
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
		(listParents.count() <= 1 && outNodeClass != UA_NODECLASS_METHOD) ||
		(listParents.count() >= 1 && outNodeClass == UA_NODECLASS_METHOD),
		"QUaServer::getParentNodeId", "Child code it not supposed to have more than one parent.");
	// return
	return listParents.count() > 0 ? listParents.at(0) : UA_NODEID_NULL;
}

// NOTE : need to cleanup result after calling this method
QList<UA_NodeId> QUaNode::getChildrenNodeIds(const UA_NodeId & parentNodeId, QUaServer * server)
{
	return QUaNode::getChildrenNodeIds(parentNodeId, server->m_server);
}

// NOTE : need to cleanup result after calling this method
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
			UA_NodeId nodeId/* = rDesc.nodeId.nodeId*/;
			UA_NodeId_copy(&rDesc.nodeId.nodeId, &nodeId);
			// ignore modelling rules
            auto nodeIdMandatory = UA_NODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY);
            if (UA_NodeId_equal(&nodeId, &nodeIdMandatory))
			{
				UA_NodeId_clear(&nodeId);
				continue;
			}
			retListChildren.append(nodeId);
		}
		UA_BrowseResult_deleteMembers(&bRes);
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
	// try to cast to C++ node, dynamic_cast check is necessary
	return qobject_cast<QUaNode*>(static_cast<QObject*>(context));
}

void * QUaNode::getVoidContext(const UA_NodeId & nodeId, UA_Server * server)
{
	// get void context
	void * context;
	auto st = UA_Server_getNodeContext(server, nodeId, &context);
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
	QString strBrowseName = QUaTypesConverter::uaStringToQString(outBrowseName.name);
	UA_QualifiedName_clear(&outBrowseName);
	return strBrowseName;
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

QDebug operator<<(QDebug debug, const QUaReferenceType& refType)
{
	QDebugStateSaver saver(debug);
	debug.nospace() << '{' << refType.strForwardName << ", " << refType.strInverseName << '}';
	return debug;
}