#include "quanode.h"

#include <QUaServer>
#include <QUaProperty>
#include <QUaBaseDataVariable>
#include <QUaFolderObject>

QOpcUaServerNode::QOpcUaServerNode(QOpcUaServer *server)
{
	Q_CHECK_PTR(server);
	Q_CHECK_PTR(server->m_newnodeNodeId);
	Q_CHECK_PTR(server->m_newNodeMetaObject);
	const UA_NodeId   &nodeId     = *server->m_newnodeNodeId;
	const QMetaObject &metaObject = *server->m_newNodeMetaObject;
	// check
	Q_ASSERT(server && !UA_NodeId_isNull(&nodeId));
	// bind itself, only good for constructors of derived classes, because UA constructor overwrites it
	// so we need to also set the context again in QOpcUaServer::uaConstructor
	// set server instance
	this->m_qopcuaserver = server;
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
	auto chidrenNodeIds = QOpcUaServerNode::getChildrenNodeIds(nodeId, server);
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
		QString strBrowseName = QOpcUaServerNode::getBrowseName(childNodeId, server);
		Q_ASSERT(!mapChildren.contains(strBrowseName));
		mapChildren[strBrowseName] = childNodeId;
	}
	// list meta props
	int propCount  = metaObject.propertyCount();
	int propOffset = QOpcUaServerNode::getPropsOffsetHelper(metaObject);
	int numProps   = 0;
	for (int i = propOffset; i < propCount; i++)
	{
		QMetaProperty metaproperty = metaObject.property(i);
		// check if not enum
		if (!metaproperty.isEnumType())
		{
			// check if available in meta-system
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
			Q_ASSERT_X(!propMetaObject.inherits(&metaObject), "QOpcUaServerNodeFactory", "Qt MetaProperty type cannot inherit from Class.");
			if (propMetaObject.inherits(&metaObject))
			{
				continue;
			}
		}
		// inc number of valid props
		numProps++;
		// the Qt meta property name must match the UA browse name
		QString strBrowseName = QString(metaproperty.name());
		Q_ASSERT(mapChildren.contains(strBrowseName));
		// get child nodeId for child
		auto childNodeId = mapChildren.take(strBrowseName);
		// get node context (C++ instance)
		auto nodeInstance = QOpcUaServerNode::getNodeContext(childNodeId, server);
		// assign C++ parent
		nodeInstance->setParent(this);
		nodeInstance->setObjectName(strBrowseName);
		// [NOTE] writing a pointer value to a Q_PROPERTY did not work, 
		//        eventhough there appear to be some success cases on the internet
		//        so in the end we have to wuery children by object name
	}
	// if assert below fails, review filter in QOpcUaServerNode::getChildrenNodeIds
	Q_ASSERT_X(mapChildren.count()      == 0        &&
		       chidrenNodeIds.count()   == numProps &&
		       this->children().count() == numProps, "QOpcUaServerNode::QOpcUaServerNode", "Children not bound properly.");
}

QString QOpcUaServerNode::displayName() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read description
	UA_LocalizedText outDisplayName;
	auto st = UA_Server_readDisplayName(m_qopcuaserver->m_server, m_nodeId, &outDisplayName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return
	return QOpcUaTypesConverter::uaStringToQString(outDisplayName.text);
	// TODO : handle outDisplayName.locale
}

void QOpcUaServerNode::setDisplayName(const QString & displayName)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDisplayName = displayName.toUtf8(); // NOTE : QByteArray must exist in stack
    UA_LocalizedText uaDisplayName = UA_LOCALIZEDTEXT((char*)"en-US", byteDisplayName.data());
	// set value
	auto st = UA_Server_writeDisplayName(m_qopcuaserver->m_server, m_nodeId, uaDisplayName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit displayName changed
	emit this->displayNameChanged(displayName);
	// TODO : handle locale
}

QString QOpcUaServerNode::description() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read description
	UA_LocalizedText outDescription;
	auto st = UA_Server_readDescription(m_qopcuaserver->m_server, m_nodeId, &outDescription);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return
	return QOpcUaTypesConverter::uaStringToQString(outDescription.text);
	// TODO : handle outDescription.locale
}

void QOpcUaServerNode::setDescription(const QString & description)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDescription = description.toUtf8(); // NOTE : QByteArray must exist in stack
	UA_LocalizedText uaDescription = UA_LOCALIZEDTEXT((char*)"en-US", byteDescription.data());
	// set value
	auto st = UA_Server_writeDescription(m_qopcuaserver->m_server, m_nodeId, uaDescription);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit description changed
	emit this->descriptionChanged(description);
	// TODO : handle locale
}

quint32 QOpcUaServerNode::writeMask() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return quint32();
	}
	// read writeMask
	UA_UInt32 outWriteMask;
	auto st = UA_Server_readWriteMask(m_qopcuaserver->m_server, m_nodeId, &outWriteMask);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// return
	return outWriteMask;
}

void QOpcUaServerNode::setWriteMask(const quint32 & writeMask)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set value
	auto st = UA_Server_writeWriteMask(m_qopcuaserver->m_server, m_nodeId, writeMask);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit writeMask changed
	emit this->writeMaskChanged(writeMask);
}

QString QOpcUaServerNode::nodeId() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	return QOpcUaTypesConverter::nodeIdToQString(m_nodeId);
}

QString QOpcUaServerNode::nodeClass() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read nodeClass
	UA_NodeClass outNodeClass;
	auto st = UA_Server_readNodeClass(m_qopcuaserver->m_server, m_nodeId, &outNodeClass);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// convert to QString
	return QOpcUaTypesConverter::nodeClassToQString(outNodeClass);
}

QString QOpcUaServerNode::browseName() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read browse name
	UA_QualifiedName outBrowseName;
	auto st = UA_Server_readBrowseName(m_qopcuaserver->m_server, m_nodeId, &outBrowseName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// populate return value
	// NOTE : ignore Namespace index outBrowseName.namespaceIndex
	return QOpcUaTypesConverter::uaStringToQString(outBrowseName.name);
}

void QOpcUaServerNode::setBrowseName(const QString & browseName)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_QualifiedName
	UA_QualifiedName bName;
	bName.namespaceIndex = 1; // NOTE : force default namespace index 1
	bName.name           = QOpcUaTypesConverter::uaStringFromQString(browseName);
	// set value
	auto st = UA_Server_writeBrowseName(m_qopcuaserver->m_server, m_nodeId, bName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// emit browseName changed
	emit this->browseNameChanged(browseName);
}

QOpcUaProperty * QOpcUaServerNode::addProperty()
{
	return m_qopcuaserver->createInstance<QOpcUaProperty>(this);
}

QOpcUaBaseDataVariable * QOpcUaServerNode::addBaseDataVariable()
{
	return m_qopcuaserver->createInstance<QOpcUaBaseDataVariable>(this);
}

QOpcUaBaseObject * QOpcUaServerNode::addBaseObject()
{
	return m_qopcuaserver->createInstance<QOpcUaBaseObject>(this);
}

QOpcUaFolderObject * QOpcUaServerNode::addFolderObject()
{
	return m_qopcuaserver->createInstance<QOpcUaFolderObject>(this);
}

UA_Server * QOpcUaServerNode::getUAServer()
{
	return m_qopcuaserver->m_server;
}

UA_NodeId QOpcUaServerNode::getParentNodeId(const UA_NodeId & childNodeId, QOpcUaServer * server)
{
	return QOpcUaServerNode::getParentNodeId(childNodeId, server->m_server);
}

UA_NodeId QOpcUaServerNode::getParentNodeId(const UA_NodeId & childNodeId, UA_Server * server)
{
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	bDesc->nodeId          = childNodeId; // from child
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
	// return
	Q_ASSERT_X(listParents.count() <= 1, "QOpcUaServer::getParentNodeId", "Child code it not supposed to have more than one parent.");
	return listParents.count() > 0 ? listParents.at(0) : UA_NODEID_NULL;
}

QList<UA_NodeId> QOpcUaServerNode::getChildrenNodeIds(const UA_NodeId & parentNodeId, QOpcUaServer * server)
{
	return QOpcUaServerNode::getChildrenNodeIds(parentNodeId, server->m_server);
}

QList<UA_NodeId> QOpcUaServerNode::getChildrenNodeIds(const UA_NodeId & parentNodeId, UA_Server * server)
{
	QList<UA_NodeId> retListChildren;
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	bDesc->nodeId          = parentNodeId; // from parent
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

QOpcUaServerNode * QOpcUaServerNode::getNodeContext(const UA_NodeId & nodeId, QOpcUaServer * server)
{
	return QOpcUaServerNode::getNodeContext(nodeId, server->m_server);
}

QOpcUaServerNode * QOpcUaServerNode::getNodeContext(const UA_NodeId & nodeId, UA_Server * server)
{
	void * context = QOpcUaServerNode::getVoidContext(nodeId, server);
	// try to cast to C++ node
	return static_cast<QOpcUaServerNode*>(context);
}

void * QOpcUaServerNode::getVoidContext(const UA_NodeId & nodeId, UA_Server * server)
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

QString QOpcUaServerNode::getBrowseName(const UA_NodeId & nodeId, QOpcUaServer * server)
{
	return QOpcUaServerNode::getBrowseName(nodeId, server->m_server);
}

QString QOpcUaServerNode::getBrowseName(const UA_NodeId & nodeId, UA_Server * server)
{
	// read browse name
	UA_QualifiedName outBrowseName;
	auto st = UA_Server_readBrowseName(server, nodeId, &outBrowseName);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	// NOTE : ignore Namespace index outBrowseName.namespaceIndex
	return QOpcUaTypesConverter::uaStringToQString(outBrowseName.name);
}

int QOpcUaServerNode::getPropsOffsetHelper(const QMetaObject & metaObject)
{
	int propOffset;
	// need to set props also inherited from base class
	if (metaObject.inherits(&QOpcUaBaseVariable::staticMetaObject))
	{
		propOffset = QOpcUaBaseVariable::staticMetaObject.propertyOffset();
	}
	else
	{
		// [NOTE] : assert below does not work, dont know why
		//Q_ASSERT(metaObject.inherits(&QOpcUaBaseObject::staticMetaObject));
		propOffset = QOpcUaBaseObject::staticMetaObject.propertyOffset();
	}
	return propOffset;
}
