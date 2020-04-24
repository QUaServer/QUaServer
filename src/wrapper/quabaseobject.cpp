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
#ifdef QT_DEBUG 
	auto obj = dynamic_cast<QUaBaseObject*>(static_cast<QObject*>(methodContext));
	Q_CHECK_PTR(obj);
#else
	auto obj = static_cast<QUaBaseObject*>(methodContext);
#endif // QT_DEBUG 
	if (!obj)
	{
		return (UA_StatusCode)UA_STATUSCODE_BADUNEXPECTEDERROR;
	}
	Q_ASSERT(obj->m_hashMethods.contains(*methodId));
	// get method from node callbacks map and call it
	return obj->m_hashMethods[*methodId](input, output);
}

QUaBaseObject::QUaBaseObject(
	QUaServer *server
) : QUaNode(server)
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

bool QUaBaseObject::subscribeToEvents() const
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	return eventNotifier.bits.bSubscribeToEvents;
}

void QUaBaseObject::setSubscribeToEvents(const bool& subscribeToEvents)
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	eventNotifier.bits.bSubscribeToEvents = subscribeToEvents;
	this->setEventNotifier(eventNotifier.intValue);
}

#ifdef UA_ENABLE_HISTORIZING

bool QUaBaseObject::eventHistoryRead() const
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	return eventNotifier.bits.bHistoryRead;
}

void QUaBaseObject::setEventHistoryRead(const bool& eventHistoryRead)
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	eventNotifier.bits.bHistoryRead = eventHistoryRead;
	this->setEventNotifier(eventNotifier.intValue);
}

bool QUaBaseObject::eventHistoryWrite() const
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	return eventNotifier.bits.bHistoryWrite;
}

void QUaBaseObject::setEventHistoryWrite(const bool& eventHistoryWrite)
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	eventNotifier.bits.bHistoryWrite = eventHistoryWrite;
	this->setEventNotifier(eventNotifier.intValue);
}
bool QUaBaseObject::eventHistoryEnabled() const
{
	QUaEventNotifier eventNotifier;
	eventNotifier.intValue = this->eventNotifier();
	return eventNotifier.bits.bHistoryRead || eventNotifier.bits.bHistoryWrite;
}
#endif // UA_ENABLE_HISTORIZING

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

void QUaBaseObject::setMethodReturnStatusCode(const UA_StatusCode& statusCode)
{
	m_qUaServer->m_methodRetStatusCode = statusCode;
}

UA_NodeId QUaBaseObject::addMethodNodeInternal(
	const QUaQualifiedName& methodName,
	const QUaNodeId& nodeId,
	const size_t& nArgs,
	UA_Argument* inputArguments,
	UA_Argument* outputArgument
)
{
	QByteArray byteMethodName = methodName.name().toUtf8();
    // add method node
    UA_MethodAttributes methAttr = UA_MethodAttributes_default;
    methAttr.executable     = true;
    methAttr.userExecutable = true;
    methAttr.description    = UA_LOCALIZEDTEXT((char *)"",
                                               byteMethodName.data());
    methAttr.displayName    = UA_LOCALIZEDTEXT((char *)"",
                                               byteMethodName.data());
    UA_NodeId user_nodeId = nodeId;
    // create callback
    UA_NodeId methNodeId;
	UA_QualifiedName browseName = methodName;
    auto st = UA_Server_addMethodNode(
		m_qUaServer->m_server,
        user_nodeId,
        m_nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		browseName,
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
	UA_NodeId_clear(&user_nodeId);
	UA_QualifiedName_clear(&browseName);
    // return new method node id
    return methNodeId;
}
