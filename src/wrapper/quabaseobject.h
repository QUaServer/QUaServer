#ifndef QUABASEOBJECT_H
#define QUABASEOBJECT_H

#include <QUaNode>

class QUaBaseObject : public QUaNode
{
    Q_OBJECT

	// Object Attributes

	Q_PROPERTY(quint8 eventNotifier READ eventNotifier WRITE setEventNotifier NOTIFY eventNotifierChanged)

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaBaseObject(
		QUaServer *server
	);

	// Attributes API

	quint8 eventNotifier() const;
	void setEventNotifier(const quint8 &eventNotifier);

	// Helpers

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	bool subscribeToEvents() const;
	void setSubscribeToEvents(const bool& subscribeToEvents);

#ifdef UA_ENABLE_HISTORIZING
	bool eventHistoryRead() const;
	void setEventHistoryRead(const bool& eventHistoryRead);

	bool eventHistoryWrite() const;
	void setEventHistoryWrite(const bool& eventHistoryWrite);

	// helper to check if either HistoryRead or HistoryWrite are set
	bool eventHistoryEnabled() const;
#endif // UA_ENABLE_HISTORIZING
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// Instance Creation API

    // NOTE : implemented in qopcuaserver.h to avoid compiler errors
	template<typename T>
	T* addChild(const QUaQualifiedName& browseName, const QUaNodeId &nodeId = QUaNodeId());

	// Method Creation API

	template<typename M>
	void addMethod(
		const QUaQualifiedName &methodName, 
		const M &methodCallback, 
		const QUaNodeId& nodeId = QUaNodeId()
	);

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// Events API

	// create instance of a given event type
	template<typename T>
	T* createEvent();

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

signals:
	void eventNotifierChanged(const quint8 &eventNotifier);

protected:
	void setMethodReturnStatusCode(const UA_StatusCode& statusCode);

private:

	UA_NodeId addMethodNodeInternal(
		const QUaQualifiedName &methodName,
		const QUaNodeId        &nodeId,
		const size_t           &nArgs,
		UA_Argument            *inputArguments,
		UA_Argument            *outputArgument
	);

	static UA_StatusCode methodCallback(
		UA_Server        *server,
		const UA_NodeId  *sessionId,
		void             *sessionContext,
		const UA_NodeId  *methodId,
		void             *methodContext,
		const UA_NodeId  *objectId,
		void             *objectContext,
		size_t            inputSize,
		const UA_Variant *input,
		size_t            outputSize,
		UA_Variant       *output
	);

	QHash< UA_NodeId, std::function<UA_StatusCode(const UA_Variant*, UA_Variant*)>> m_hashMethods;
};

#endif // QUABASEOBJECT_H

