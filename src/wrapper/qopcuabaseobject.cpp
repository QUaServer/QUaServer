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
	Q_UNUSED(server        );
	Q_UNUSED(sessionId     );
	Q_UNUSED(sessionContext);
	Q_UNUSED(objectId      );
	Q_UNUSED(objectContext );
	Q_UNUSED(inputSize     );
	Q_UNUSED(outputSize    );
	// get node from context object
	auto obj = dynamic_cast<QOpcUaBaseObject*>(static_cast<QObject*>(methodContext));
	Q_CHECK_PTR(obj);
	if (!obj)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	Q_ASSERT(obj->m_hashMethods.contains(*methodId));
	// get method from node callbacks map and call it
	return obj->m_hashMethods[*methodId](input, output);
}

QOpcUaBaseObject::QOpcUaBaseObject(QOpcUaServer *server)
	: QOpcUaServerNode(server)
{

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