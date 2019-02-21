#include "qopcuabaseobject.h"

#include <QOpcUaServer>
#include <QOpcUaProperty>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaFolderObject>

QOpcUaBaseObject::QOpcUaBaseObject(QOpcUaServerNode *parent) : QOpcUaServerNode(parent)
{
	// Set NodeId manually only for pre-defined types. 
	// For custom types, the type NodeId is set upon registering the class in the server
}

// PROTECTED
QOpcUaBaseObject::QOpcUaBaseObject(QOpcUaServer * server) : QOpcUaServerNode(server)
{
	// This is a private constructor meant to be called only by QOpcUaServerNode
	// And its only purpose is to create the UA_NS0ID_OBJECTSFOLDER instance
}

QOpcUaProperty * QOpcUaBaseObject::addProperty(const QString &strBrowseName/* = ""*/)
{
	return m_qopcuaserver->createInstance<QOpcUaProperty>(this, strBrowseName);
}

QOpcUaBaseDataVariable * QOpcUaBaseObject::addBaseDataVariable(const QString &strBrowseName/* = ""*/)
{
	return m_qopcuaserver->createInstance<QOpcUaBaseDataVariable>(this, strBrowseName);
}

QOpcUaBaseObject * QOpcUaBaseObject::addBaseObject(const QString &strBrowseName/* = ""*/)
{
	return m_qopcuaserver->createInstance<QOpcUaBaseObject>(this, strBrowseName);
}

QOpcUaFolderObject * QOpcUaBaseObject::addFolderObject(const QString &strBrowseName/* = ""*/)
{
	return m_qopcuaserver->createInstance<QOpcUaFolderObject>(this, strBrowseName);
}
