#include "qopcuafolderobject.h"

QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServerNode *parent) : QOpcUaBaseObject(parent)
{
	// Set NodeId manually only for pre-defined types. 
	// For custom types, the type NodeId is set upon registering the class in the server
	m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
}

QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServer *server) : QOpcUaBaseObject(server)
{
	// This is a private constructor meant to be called only by QOpcUaServerNode
	// And its only purpose is to create the UA_NS0ID_OBJECTSFOLDER instance
	m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
	m_nodeId     = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
}