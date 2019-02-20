#include "qopcuafolderobject.h"

#include <QOpcUaBaseObject>
#include <QOpcUaBaseDataVariable>

// NOTE : define typeNodeId
UA_NodeId QOpcUaNodeFactory<QOpcUaFolderObject>::m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);

QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServerNode *parent) : QOpcUaAbstractObject(parent)
{
	// Set NodeId manually only for pre-defined types. 
	// For custom types, the type NodeId is set upon registering the class in the server
}

// PRIVATE
QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServer *server) : QOpcUaAbstractObject(server)
{
	// This is a private constructor meant to be called only by QOpcUaServerNode
	// And its only purpose is to create the UA_NS0ID_OBJECTSFOLDER instance
	//m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
	m_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
}

QOpcUaBaseObject * QOpcUaFolderObject::addBaseObject()
{
	return m_qopcuaserver->createInstance<QOpcUaBaseObject>(this);
}

QOpcUaBaseDataVariable * QOpcUaFolderObject::addBaseDataVariable()
{
	return m_qopcuaserver->createInstance<QOpcUaBaseDataVariable>(this);
}

QOpcUaFolderObject * QOpcUaFolderObject::addFolderObject()
{
	return m_qopcuaserver->createInstance<QOpcUaFolderObject>(this);
}

