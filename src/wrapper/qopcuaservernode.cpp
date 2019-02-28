#include "qopcuaservernode.h"

#include <QOpcUaServer>
#include <QOpcUaProperty>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaFolderObject>

#include "mynewvariabletype.h"

QOpcUaServerNode::QOpcUaServerNode(QOpcUaServerNode *parent) : QObject(parent)
{
	m_qopcuaserver = parent->m_qopcuaserver;
	m_nodeId       = UA_NODEID_NULL;
	// NOTE : type NodeId is null until class is registered in server
	Q_CHECK_PTR(m_qopcuaserver);
}

// PROTECTED
QOpcUaServerNode::QOpcUaServerNode(QOpcUaServer * server) : QObject(server)
{
	// This is a private constructor meant to be called only by QOpcUaServerNode
	// And its only purpose is to create the UA_NS0ID_OBJECTSFOLDER instance
	m_qopcuaserver = server;
	m_nodeId       = UA_NODEID_NULL;
	// NOTE : type NodeId is null until class is registered in server
	Q_CHECK_PTR(m_qopcuaserver);
}

QString QOpcUaServerNode::get_displayName() const
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

void QOpcUaServerNode::set_displayName(const QString & displayName)
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

QString QOpcUaServerNode::get_description() const
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

void QOpcUaServerNode::set_description(const QString & description)
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

quint32 QOpcUaServerNode::get_writeMask() const
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

void QOpcUaServerNode::set_writeMask(const quint32 & writeMask)
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

QString QOpcUaServerNode::get_nodeId() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	return QOpcUaTypesConverter::nodeIdToQString(m_nodeId);
}

QString QOpcUaServerNode::get_nodeClass() const
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

QString QOpcUaServerNode::get_browseName() const
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

void QOpcUaServerNode::set_browseName(const QString & browseName)
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
