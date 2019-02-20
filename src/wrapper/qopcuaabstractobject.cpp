#include "qopcuaabstractobject.h"

QOpcUaAbstractObject::QOpcUaAbstractObject(QOpcUaServerNode *parent) : QOpcUaServerNode(parent)
{
	// Set NodeId manually only for pre-defined types. 
	// For custom types, the type NodeId is set upon registering the class in the server
}

// PROTECTED
QOpcUaAbstractObject::QOpcUaAbstractObject(QOpcUaServer *server) : QOpcUaServerNode(server)
{
	// This is a private constructor meant to be called only by QOpcUaServerNode
	// And its only purpose is to create the UA_NS0ID_OBJECTSFOLDER instance
}