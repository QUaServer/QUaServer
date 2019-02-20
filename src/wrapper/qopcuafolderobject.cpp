#include "qopcuafolderobject.h"

QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServerNode *parent) : QOpcUaBaseObject(parent)
{
	// Set NodeId manually only for pre-defined types. 
	// For custom types, the type NodeId is set upon registering the class in the server
}

// PRIVATE
QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServer *server) : QOpcUaBaseObject(server)
{
	// This is a private constructor meant to be called only by QOpcUaServerNode
	// And its only purpose is to create the UA_NS0ID_OBJECTSFOLDER instance
	/*
	This standard Object is the browse entry point for Object Nodes. 
	Only Organizes References are used to relate Objects to the "Objects" standard Object.
	The intent of the “Objects” Object is that all Objects and Variables that are not used for 
	type definitions or other organizational purposes (e.g. organizing the Views) are accessible 
	through hierarchical References starting from this Node. 
	However, this is not a requirement, because not all Servers may be able to support this.
	*/
	m_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
}
