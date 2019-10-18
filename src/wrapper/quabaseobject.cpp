#include "quabaseobject.h"

#include <QUaServer>

// [STATIC]
UA_StatusCode QUaBaseObject::methodCallback(UA_Server        * server,
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
	auto obj = dynamic_cast<QUaBaseObject*>(static_cast<QObject*>(methodContext));
	Q_CHECK_PTR(obj);
	if (!obj)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	Q_ASSERT(obj->m_hashMethods.contains(*methodId));
	// get method from node callbacks map and call it
	return obj->m_hashMethods[*methodId](input, output);
}

QUaBaseObject::QUaBaseObject(QUaServer *server)
	: QUaNode(server)
{

}

quint8 QUaBaseObject::eventNotifier() const
{
	UA_Byte outByte;
	auto st = UA_Server_readEventNotifier(m_qUaServer->m_server, m_nodeId, &outByte);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return outByte;
}

void QUaBaseObject::setEventNotifier(const quint8 & eventNotifier)
{
	auto st = UA_Server_writeEventNotifier(m_qUaServer->m_server, m_nodeId, eventNotifier);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	emit this->eventNotifierChanged(eventNotifier);
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

void QUaBaseObject::setEventNotifierSubscribeToEvents()
{
	this->setEventNotifier(UA_EVENTNOTIFIERTYPE_SUBSCRIBETOEVENTS);
}

void QUaBaseObject::setEventNotifierNone()
{
	this->setEventNotifier(UA_EVENTNOTIFIERTYPE_NONE);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

UA_NodeId QUaBaseObject::addMethodNodeInternal(QByteArray &byteMethodName, const QString &strNodeId, const size_t &nArgs, UA_Argument * inputArguments, UA_Argument * outputArgument)
{
    // add method node
    UA_MethodAttributes methAttr = UA_MethodAttributes_default;
    methAttr.executable     = true;
    methAttr.userExecutable = true;
    methAttr.description    = UA_LOCALIZEDTEXT((char *)"",
                                               byteMethodName.data());
    methAttr.displayName    = UA_LOCALIZEDTEXT((char *)"",
                                               byteMethodName.data());
    UA_NodeId   user_nodeId = strNodeId.isEmpty() ? UA_NODEID_NULL : QUaTypesConverter::nodeIdFromQString(strNodeId);
    // create callback
    UA_NodeId methNodeId;
    auto st = UA_Server_addMethodNode(m_qUaServer->m_server,
                                      user_nodeId,
                                      m_nodeId,
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                      UA_QUALIFIEDNAME (1, byteMethodName.data()),
                                      methAttr,
                                      &QUaBaseObject::methodCallback,
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
