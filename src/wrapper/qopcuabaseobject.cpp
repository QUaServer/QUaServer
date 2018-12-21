#include "qopcuabaseobject.h"

UA_NodeId QOpcUaBaseObject::m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);

QOpcUaBaseObject::QOpcUaBaseObject(QOpcUaServerNode *parent) : QOpcUaServerNode(parent)
{
	// Set NodeId manually only for pre-defined types. 
	// For custom types, the type NodeId is set upon registering the class in the server
	//m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
}

QOpcUaBaseObject::QOpcUaBaseObject(QOpcUaServer *server) : QOpcUaServerNode(server)
{

}