#include "qopcuaservernode.h"
#include <QOpcUaServer>

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
	UA_Server_readDisplayName(m_qopcuaserver->m_server, m_nodeId, &outDisplayName);
	return QOpcUaTypesConverter::uaStringToQString(outDisplayName.text);
	// TODO : handle outDisplayName.locale
}

void QOpcUaServerNode::set_displayName(const QString & displayName)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDisplayName = displayName.toUtf8(); // NOTE : QByteArray must exist in stack
    UA_LocalizedText uaDisplayName = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
	// set value
	UA_Server_writeDisplayName(m_qopcuaserver->m_server, m_nodeId, uaDisplayName);
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
	UA_Server_readDescription(m_qopcuaserver->m_server, m_nodeId, &outDescription);
	return QOpcUaTypesConverter::uaStringToQString(outDescription.text);
	// TODO : handle outDescription.locale
}

void QOpcUaServerNode::set_description(const QString & description)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDescription = description.toUtf8(); // NOTE : QByteArray must exist in stack
	UA_LocalizedText uaDescription = UA_LOCALIZEDTEXT((char*)"", byteDescription.data());
	// set value
	UA_Server_writeDescription(m_qopcuaserver->m_server, m_nodeId, uaDescription);
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
	UA_Server_readWriteMask(m_qopcuaserver->m_server, m_nodeId, &outWriteMask);
	return outWriteMask;
}

void QOpcUaServerNode::set_writeMask(const quint32 & writeMask)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set value
	UA_Server_writeWriteMask(m_qopcuaserver->m_server, m_nodeId, writeMask);
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
	UA_Server_readNodeClass(m_qopcuaserver->m_server, m_nodeId, &outNodeClass);
	// convert to QString
	return QOpcUaTypesConverter::nodeClassToQString(outNodeClass);
}

QPair<quint16, QString> QOpcUaServerNode::get_browseName() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QPair<quint16, QString>();
	}
	// read browse name
	UA_QualifiedName outBrowseName;
	UA_Server_readBrowseName(m_qopcuaserver->m_server, m_nodeId, &outBrowseName);
	// populate return value
	return QPair<quint16, QString>(outBrowseName.namespaceIndex, QOpcUaTypesConverter::uaStringToQString(outBrowseName.name));
}

void QOpcUaServerNode::set_browseName(const QBrowseName & browseName)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_QualifiedName
	UA_QualifiedName bName;
	bName.namespaceIndex = browseName.first;
	bName.name           = QOpcUaTypesConverter::uaStringFromQString(browseName.second);
	// set value
	UA_Server_writeBrowseName(m_qopcuaserver->m_server, m_nodeId, bName);
}

