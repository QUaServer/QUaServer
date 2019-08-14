#ifndef QUABASEOBJECT_H
#define QUABASEOBJECT_H

#include <QUaNode>

/*
typedef struct {                          // UA_ObjectTypeAttributes_default
	// Node Attributes
	UA_UInt32        specifiedAttributes; // 0,
	UA_LocalizedText displayName;         // {{0, NULL}, {0, NULL}},
	UA_LocalizedText description;         // {{0, NULL}, {0, NULL}},
	UA_UInt32        writeMask;           // 0,
	UA_UInt32        userWriteMask;       // 0,
	// Object Type Attributes
	UA_Boolean       isAbstract;          // false
} UA_ObjectTypeAttributes;

typedef struct {                          // UA_ObjectAttributes_default
	// Node Attributes
	UA_UInt32        specifiedAttributes; // 0,
	UA_LocalizedText displayName;         // {{0, NULL}, {0, NULL}},
	UA_LocalizedText description;         // {{0, NULL}, {0, NULL}},
	UA_UInt32        writeMask;           // 0,
	UA_UInt32        userWriteMask;       // 0,
	// Object Attributes
	UA_Byte          eventNotifier;       // 0
} UA_ObjectAttributes;
*/

// Part 5 - 6.2 : BaseObjectType
/*
The BaseObjectType is used as type definition whenever there is an Object 
having no more concrete type definitions available. 
Servers should avoid using this ObjectType and use a more specific type, if possible. 
This ObjectType is the base ObjectType and all other ObjectTypes shall either 
directly or indirectly inherit from it.
*/

class QUaBaseObject : public QUaNode
{
    Q_OBJECT

	// Object Attributes

	Q_PROPERTY(quint8 eventNotifier READ eventNotifier WRITE setEventNotifier NOTIFY eventNotifierChanged)

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaBaseObject(QUaServer *server);

	// Attributes API

	quint8 eventNotifier() const;
	void setEventNotifier(const quint8 &eventNotifier);

	// Helpers

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	void setEventNotifierSubscribeToEvents();
	void setEventNotifierNone();
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// Instance Creation API

    // NOTE : implemented in qopcuaserver.h to avoid compiler errors
	template<typename T>
	T* addChild(const QString &strNodeId = "");

	// Method Creation API

	template<typename M>
	void addMethod(const QString &strMethodName, const M &methodCallback);

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// Events API

	// create instance of a given event type
	template<typename T>
	T* createEvent();

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

signals:
	void eventNotifierChanged(const quint8 &eventNotifier);

private:

    UA_NodeId addMethodNodeInternal(QByteArray   &byteMethodName, 
		                            const size_t &nArgs, 
		                            UA_Argument  *inputArguments, 
		                            UA_Argument  *outputArgument);

	static UA_StatusCode methodCallback(UA_Server        *server,
		                                const UA_NodeId  *sessionId,
		                                void             *sessionContext,
		                                const UA_NodeId  *methodId,
		                                void             *methodContext,
		                                const UA_NodeId  *objectId,
		                                void             *objectContext,
		                                size_t            inputSize,
		                                const UA_Variant *input,
		                                size_t            outputSize,
		                                UA_Variant       *output);

	QHash< UA_NodeId, std::function<UA_StatusCode(const UA_Variant*, UA_Variant*)>> m_hashMethods;
};

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
template<typename T>
inline T * QUaBaseObject::createEvent()
{
	const QStringList * defaultProperties = getDefaultPropertiesRef<T>();
	Q_ASSERT(defaultProperties);
	// instantiate first in OPC UA
	UA_NodeId newEventNodeId = m_qUaServer->createEvent(T::staticMetaObject, m_nodeId, defaultProperties);
	if (UA_NodeId_isNull(&newEventNodeId))
	{
		return nullptr;
	}
	// get new c++ instance created in UA constructor
	auto tmp = QUaNode::getNodeContext(newEventNodeId, m_qUaServer);
	T * newEvent = dynamic_cast<T*>(tmp);
	Q_CHECK_PTR(newEvent);
	// return c++ event instance
	return newEvent;
}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUABASEOBJECT_H

