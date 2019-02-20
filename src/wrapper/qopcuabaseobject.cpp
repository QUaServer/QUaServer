#include "qopcuabaseobject.h"

#include <QOpcUaBaseDataVariable>
#include <QOpcUaFolderObject>

QOpcUaBaseObject::QOpcUaBaseObject(QOpcUaServerNode *parent) : QOpcUaAbstractObject(parent)
{
	// Set NodeId manually only for pre-defined types. 
	// For custom types, the type NodeId is set upon registering the class in the server
}

// PROTECTED
QOpcUaBaseObject::QOpcUaBaseObject(QOpcUaServer * server) : QOpcUaAbstractObject(server)
{
	// This is a private constructor meant to be called only by QOpcUaServerNode
	// And its only purpose is to create the UA_NS0ID_OBJECTSFOLDER instance
}

QOpcUaBaseObject * QOpcUaBaseObject::addBaseObject()
{
	return m_qopcuaserver->createInstance<QOpcUaBaseObject>(this);
}

QOpcUaBaseDataVariable * QOpcUaBaseObject::addBaseDataVariable()
{
	return m_qopcuaserver->createInstance<QOpcUaBaseDataVariable>(this);
}

QOpcUaFolderObject * QOpcUaBaseObject::addFolderObject()
{
	return m_qopcuaserver->createInstance<QOpcUaFolderObject>(this);
}
