#include "qopcuabaseobject.h"

#include <QOpcUaServer>

// [STATIC]
UA_StatusCode QOpcUaBaseObject::methodCallback(UA_Server        * server,
	                                           const UA_NodeId  * sessionId, 
	                                           void             * sessionContext, 
	                                           const UA_NodeId  * methodId, 
	                                           void             * methodContext, 
	                                           const UA_NodeId  * objectId, 
	                                           void             * objectContext, 
	                                           size_t             inputSize, 
	                                           const UA_Variant * input, 
	                                           size_t             outputSize, 
	                                           UA_Variant       * output)
{
	// get node from context object
	auto obj = static_cast<QOpcUaBaseObject*>(methodContext);
	Q_CHECK_PTR(obj);
	Q_ASSERT(obj->m_hashCallbacks.contains(*methodId));
	// get method from node callbacks map and call it
	return obj->m_hashCallbacks[*methodId](server,
		                                   sessionId, 
		                                   sessionContext, 
		                                   methodId, 
		                                   methodContext, 
		                                   objectId, 
		                                   objectContext, 
		                                   inputSize, 
		                                   input, 
		                                   outputSize, 
		                                   output);
}

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

UA_NodeId QOpcUaBaseObject::addMethodNodeInternal(QByteArray &byteMethodName, const size_t &nArgs, UA_Argument * inputArguments, UA_Argument * outputArgument)
{
    // add method node
    UA_MethodAttributes methAttr = UA_MethodAttributes_default;
    methAttr.executable     = true;
    methAttr.userExecutable = true;
    methAttr.description    = UA_LOCALIZEDTEXT((char *)"en-US",
                                               byteMethodName.data());
    methAttr.displayName    = UA_LOCALIZEDTEXT((char *)"en-US",
                                               byteMethodName.data());
    // create callback
    UA_NodeId methNodeId;
    auto st = UA_Server_addMethodNode(m_qopcuaserver->m_server,
                                      UA_NODEID_NULL,
                                      m_nodeId,
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                      UA_QUALIFIEDNAME (1, byteMethodName.data()),
                                      methAttr,
                                      &QOpcUaBaseObject::methodCallback,
                                      nArgs,
                                      inputArguments,
		                              outputArgument ? 1 : 0,
                                      outputArgument,
                                      this,
                                      &methNodeId);
    Q_ASSERT(st == UA_STATUSCODE_GOOD);
    Q_UNUSED(st);
    // return new methos node id
    return methNodeId;
}