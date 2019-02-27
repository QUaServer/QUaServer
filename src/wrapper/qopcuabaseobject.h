#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QOpcUaServerNode>

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

inline bool operator==(const UA_NodeId &e1, const UA_NodeId &e2)
{
	return e1.namespaceIndex == e2.namespaceIndex
		&& e1.identifierType == e2.identifierType
		&& e1.identifier.numeric == e2.identifier.numeric;
}

inline uint qHash(const UA_NodeId &key, uint seed)
{
	return qHash(key.namespaceIndex, seed) ^ qHash(key.identifierType, seed) ^ qHash(key.identifier.numeric, seed);
}

class QOpcUaBaseObject : public QOpcUaServerNode
{
    Q_OBJECT

	// Object Attributes

	// TODO
	//Q_PROPERTY(UA_Byte eventNotifier READ get_eventNotifier)

friend class QOpcUaServer;

public:
	explicit QOpcUaBaseObject(QOpcUaServerNode *parent);

	// Instance Creation API
    // NOTE : implemented in qopcuaserver.h to avoid compiler errors
	template<typename T>
	T* addChild();

	// Method Creation API

	template<typename RA, typename T>
	void addMethod(const QString &strMethodName, const T &methodCallback);
	// specialization, no args no return
	void addMethod(const QString &strMethodName, const std::function<void(void)> &methodCallback);

protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaBaseObject(QOpcUaServer *server);

private:

	template<typename R, typename ...A>
	void addMethodInternal(const QString &strMethodName, const std::function<R(A...)> &methodCallback);
	// specialization no args
	template<typename R>
	void addMethodInternal(const QString &strMethodName, const std::function<R(void)> &methodCallback);
	// specialization no return
	template<typename ...A>
	void addMethodInternal(const QString &strMethodName, const std::function<void(A...)> &methodCallback);
	// specialization no args and no return
	void addMethodInternal(const QString &strMethodName, const std::function<void(void)> &methodCallback);

	template<typename A>
	UA_Argument processArgType(const int &iArg);

	template<typename A>
	static A convertArgType(const UA_Variant * input, const int &iArg);

    UA_NodeId addMethodNodeInternal(QByteArray   &byteMethodName, 
		                            const size_t &nArgs, 
		                            UA_Argument  * inputArguments, 
		                            UA_Argument  * outputArgument);

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

	QHash< UA_NodeId, std::function<UA_StatusCode(UA_Server*,
		                                          const UA_NodeId*, 
		                                          void*, 
		                                          const 
		                                          UA_NodeId*, 
		                                          void*, 
		                                          const UA_NodeId*, 
		                                          void*, size_t, 
		                                          const UA_Variant*, 
		                                          size_t, 
		                                          UA_Variant*)>> m_hashCallbacks;
};

template<typename RA, typename T>
inline void QOpcUaBaseObject::addMethod(const QString & strMethodName, const T &methodCallback)
{
	return this->addMethodInternal(strMethodName, (std::function<RA>)methodCallback);
}
// specialization, no args no return
inline void QOpcUaBaseObject::addMethod(const QString &strMethodName, const std::function<void(void)> &methodCallback)
{
	return this->addMethodInternal(strMethodName, methodCallback);
}

template<typename R, typename ...A>
inline void QOpcUaBaseObject::addMethodInternal(const QString & strMethodName, const std::function<R(A...)> &methodCallback)
{
	QByteArray byteMethodName = strMethodName.toUtf8();
	// create output argument
	UA_Argument outputArgument;
	UA_Argument_init(&outputArgument);
	outputArgument.description = UA_LOCALIZEDTEXT((char *)"en-US",
		                                          (char *)"Result Value");
	outputArgument.name        = QOpcUaTypesConverter::uaStringFromQString((char *)"Result");
	outputArgument.dataType    = QOpcUaTypesConverter::uaTypeNodeIdFromCpp<R>();
	outputArgument.valueRank   = UA_VALUERANK_SCALAR;
	// create input arguments
	int iArg = 0;
	const size_t nArgs = sizeof...(A);
	UA_Argument inputArguments[nArgs] = { this->processArgType<A>(iArg++)... };
	// add method node
    UA_NodeId methNodeId = this->addMethodNodeInternal(byteMethodName, nArgs, inputArguments, &outputArgument);
	// store method with node id hash as key
	Q_ASSERT_X(!m_hashCallbacks.contains(methNodeId), "QOpcUaBaseObject::addMethodInternal", "Method already exists, callback will be overwritten.");
	m_hashCallbacks[methNodeId] = [methodCallback](UA_Server        * server,
                                                   const UA_NodeId  * sessionId,
                                                   void             * sessionContext,
                                                   const UA_NodeId  * methodId,
                                                   void             * methodContext,
                                                   const UA_NodeId  * objectId,
                                                   void             * objectContext,
                                                   size_t             inputSize,
                                                   const UA_Variant * input,
                                                   size_t             outputSize,
                                                   UA_Variant       * output) {
		// call method
		// NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
		int iArg = sizeof...(A) -1;
		QVariant varResult = methodCallback(QOpcUaBaseObject::convertArgType<A>(input, iArg--)...);
		// set result
		*output = QOpcUaTypesConverter::uaVariantFromQVariant(varResult);
		return UA_STATUSCODE_GOOD;
	};
}
// specialization no args
template<typename R>
inline void QOpcUaBaseObject::addMethodInternal(const QString &strMethodName, const std::function<R(void)> &methodCallback)
{
	QByteArray byteMethodName = strMethodName.toUtf8();
	// create output argument
	UA_Argument outputArgument;
	UA_Argument_init(&outputArgument);
	outputArgument.description = UA_LOCALIZEDTEXT((char *)"en-US",
		                                          (char *)"Result Value");
	outputArgument.name        = QOpcUaTypesConverter::uaStringFromQString((char *)"Result");
	outputArgument.dataType    = QOpcUaTypesConverter::uaTypeNodeIdFromCpp<R>();
	outputArgument.valueRank   = UA_VALUERANK_SCALAR;
	// add method node
    UA_NodeId methNodeId = this->addMethodNodeInternal(byteMethodName, 0, nullptr, &outputArgument);
	// store method with node id hash as key
	Q_ASSERT_X(!m_hashCallbacks.contains(methNodeId), "QOpcUaBaseObject::addMethodInternal", "Method already exists, callback will be overwritten.");
	m_hashCallbacks[methNodeId] = [methodCallback](UA_Server        * server,
                                                   const UA_NodeId  * sessionId,
                                                   void             * sessionContext,
                                                   const UA_NodeId  * methodId,
                                                   void             * methodContext,
                                                   const UA_NodeId  * objectId,
                                                   void             * objectContext,
                                                   size_t             inputSize,
                                                   const UA_Variant * input,
                                                   size_t             outputSize,
                                                   UA_Variant       * output) {
		// call method with no arguments
		QVariant varResult = methodCallback();
		// set result
		*output = QOpcUaTypesConverter::uaVariantFromQVariant(varResult);
		return UA_STATUSCODE_GOOD;
	};
}
// specialization no return
template<typename ...A>
inline void QOpcUaBaseObject::addMethodInternal(const QString &strMethodName, const std::function<void(A...)> &methodCallback)
{
	QByteArray byteMethodName = strMethodName.toUtf8();
	// create input arguments
	int iArg = 0;
	const size_t nArgs = sizeof...(A);
	UA_Argument inputArguments[nArgs] = { this->processArgType<A>(iArg++)... };
	// add method node
    UA_NodeId methNodeId = this->addMethodNodeInternal(byteMethodName, nArgs, inputArguments, nullptr);
	// store method with node id hash as key
	Q_ASSERT_X(!m_hashCallbacks.contains(methNodeId), "QOpcUaBaseObject::addMethodInternal", "Method already exists, callback will be overwritten.");
	m_hashCallbacks[methNodeId] = [methodCallback](UA_Server        * server,
                                                   const UA_NodeId  * sessionId,
                                                   void             * sessionContext,
                                                   const UA_NodeId  * methodId,
                                                   void             * methodContext,
                                                   const UA_NodeId  * objectId,
                                                   void             * objectContext,
                                                   size_t             inputSize,
                                                   const UA_Variant * input,
                                                   size_t             outputSize,
                                                   UA_Variant       * output) {
		// call method
		// NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
		int iArg = sizeof...(A) -1;
		methodCallback(QOpcUaBaseObject::convertArgType<A>(input, iArg--)...);
		// no result
		return UA_STATUSCODE_GOOD;
	};
}
// specialization no args and no return
inline void QOpcUaBaseObject::addMethodInternal(const QString &strMethodName, const std::function<void(void)> &methodCallback)
{
	QByteArray byteMethodName = strMethodName.toUtf8();
	// add method node
    UA_NodeId methNodeId = this->addMethodNodeInternal(byteMethodName, 0, nullptr, nullptr);
	// store method with node id hash as key
	Q_ASSERT_X(!m_hashCallbacks.contains(methNodeId), "QOpcUaBaseObject::addMethodInternal", "Method already exists, callback will be overwritten.");
	m_hashCallbacks[methNodeId] = [methodCallback](UA_Server        * server,
                                                   const UA_NodeId  * sessionId,
                                                   void             * sessionContext,
                                                   const UA_NodeId  * methodId,
                                                   void             * methodContext,
                                                   const UA_NodeId  * objectId,
                                                   void             * objectContext,
                                                   size_t             inputSize,
                                                   const UA_Variant * input,
                                                   size_t             outputSize,
                                                   UA_Variant       * output) {
		// call method
		methodCallback();
		// no result
		return UA_STATUSCODE_GOOD;
	};
}


template<typename A>
inline UA_Argument QOpcUaBaseObject::processArgType(const int &iArg)
{
	UA_Argument inputArgument;
	UA_Argument_init(&inputArgument);
	// create n-th argument with name "Arg" + number
	inputArgument.description = UA_LOCALIZEDTEXT((char *)"en-US", (char *)"Method Argument");
	inputArgument.name        = QOpcUaTypesConverter::uaStringFromQString(trUtf8("Arg%1").arg(iArg));
	inputArgument.dataType    = QOpcUaTypesConverter::uaTypeNodeIdFromCpp<A>();
	inputArgument.valueRank   = UA_VALUERANK_SCALAR;
	// return
	return inputArgument;
}

template<typename A>
inline A QOpcUaBaseObject::convertArgType(const UA_Variant * input, const int &iArg)
{
	QVariant varQt = QOpcUaTypesConverter::uaVariantToQVariant(input[iArg]);
	//qDebug() << "iArg =" << iArg << ", varQt =" << varQt;
	return varQt.value<A>();
}

#endif // QOPCUABASEOBJECT_H

