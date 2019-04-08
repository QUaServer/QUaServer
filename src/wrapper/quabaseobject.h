#ifndef QUABASEOBJECT_H
#define QUABASEOBJECT_H

#include <QUaNode>

template <typename ClassType, typename R, bool IsMutable, typename... Args>
struct QUaMethodTraitsBase
{
	inline static bool getIsMutable()
	{
		return IsMutable;
	}

	inline static const size_t getNumArgs()
	{
		return sizeof...(Args);
	}

	template<typename T>
	inline static QString getTypeName()
	{
		return QString(typeid(T).name());
	}

	inline static QString getRetType()
	{
		return getTypeName<R>();
	}

	inline static QStringList getArgTypes()
	{
		return { getTypeName<Args>()... };
	}

	template<typename T>
	inline static UA_Argument getTypeUaArgument(const int &iArg = 0)
	{
		UA_Argument inputArgument;
		UA_Argument_init(&inputArgument);
		// create n-th argument with name "Arg" + number
		inputArgument.description = UA_LOCALIZEDTEXT((char *)"", (char *)"Method Argument");
		inputArgument.name        = QUaTypesConverter::uaStringFromQString(QObject::trUtf8("Arg%1").arg(iArg));
		inputArgument.dataType    = QUaTypesConverter::uaTypeNodeIdFromCpp<T>();
		inputArgument.valueRank   = UA_VALUERANK_SCALAR;
		// return
		return inputArgument;
	}

	inline static const bool isRetUaArgumentVoid()
	{
		return std::is_same<R, void>::value;
	}

	inline static UA_Argument getRetUaArgument()
	{
		if (isRetUaArgumentVoid()) return UA_Argument();
		// create output argument
		UA_Argument outputArgument;
		UA_Argument_init(&outputArgument);
		outputArgument.description = UA_LOCALIZEDTEXT((char *)"",
													  (char *)"Result Value");
		outputArgument.name        = QUaTypesConverter::uaStringFromQString((char *)"Result");
		outputArgument.dataType    = QUaTypesConverter::uaTypeNodeIdFromCpp<R>();
		outputArgument.valueRank   = UA_VALUERANK_SCALAR;
		return outputArgument;
	}

	inline static QVector<UA_Argument> getArgsUaArguments()
	{
		int iArg = 0;
		const size_t nArgs = getNumArgs();
		if (nArgs <= 0) return QVector<UA_Argument>();
		return { getTypeUaArgument<Args>(iArg++)... };
	}

	template<typename T>
	inline static T convertArgType(const UA_Variant * input, const int &iArg)
	{
		QVariant varQt = QUaTypesConverter::uaVariantToQVariant(input[iArg]);
		return varQt.value<T>();
	}

    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
	{
		// call method
		// NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
		int iArg = (int)getNumArgs() - 1;
		// call method
		QVariant varResult = methodCallback(convertArgType<Args>(input, iArg--)...);
		// set result
		UA_Variant retVar = QUaTypesConverter::uaVariantFromQVariant(varResult);

		// TODO : cleanup? UA_Variant_deleteMembers(&retVar);

		return retVar;
	}
};
// general case
template<typename T>
struct QOpcUaMethodTraits : QOpcUaMethodTraits<decltype(&T::operator())>
{};
// specialization - const 
template <typename ClassType, typename R, typename... Args>
struct QOpcUaMethodTraits< R(ClassType::*)(Args...) const > : QUaMethodTraitsBase<ClassType, R, false, Args...>
{};
// specialization - mutable 
template <typename ClassType, typename R, typename... Args>
struct QOpcUaMethodTraits< R(ClassType::*)(Args...) > : QUaMethodTraitsBase<ClassType, R, true, Args...>
{};
// specialization - const | no return value
template <typename ClassType, typename... Args>
struct QOpcUaMethodTraits< void(ClassType::*)(Args...) const > : QUaMethodTraitsBase<ClassType, void, false, Args...>
{
    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
	{
		// call method
		// NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
        int iArg = (int)QOpcUaMethodTraits<M>::getNumArgs() - 1;
		// call method
        methodCallback(QOpcUaMethodTraits<M>::template convertArgType<Args>(input, iArg--)...);
		// set result
		return UA_Variant();
	}
};
// specialization - mutable | no return value
template <typename ClassType, typename... Args>
struct QOpcUaMethodTraits< void(ClassType::*)(Args...) > : QUaMethodTraitsBase<ClassType, void, true, Args...>
{
    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
	{
		// call method
		// NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
        int iArg = QOpcUaMethodTraits<M>::getNumArgs() - 1;
		// call method
        methodCallback(QOpcUaMethodTraits<M>::template convertArgType<Args>(input, iArg--)...);
		// set result
		return UA_Variant();
	}
};
// specialization - function pointer
template <typename R, typename... Args>
struct QOpcUaMethodTraits< R(*)(Args...) > : QUaMethodTraitsBase<void, R, true, Args...>
{};
// specialization - function pointer | no return value
template <typename... Args>
struct QOpcUaMethodTraits< void(*)(Args...) > : QUaMethodTraitsBase<void, void, true, Args...>
{
	template<typename M>
	inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
	{
		// call method
		// NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
		int iArg = QOpcUaMethodTraits<M>::getNumArgs() - 1;
		// call method
		methodCallback(QOpcUaMethodTraits<M>::template convertArgType<Args>(input, iArg--)...);
		// set result
		return UA_Variant();
	}
};

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

	//Q_PROPERTY(UA_Byte eventNotifier READ eventNotifier)
	// UA_Server_readEventNotifier
	// UA_Server_writeEventNotifier

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaBaseObject(QUaServer *server);

	// Attributes API

	// TODO : eventNotifier

	// Instance Creation API

    // NOTE : implemented in qopcuaserver.h to avoid compiler errors
	template<typename T>
	T* addChild(const QString &strNodeId = "");

	// Method Creation API

	template<typename M>
	void addMethod(const QString &strMethodName, const M &methodCallback);

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

template<typename M>
inline void QUaBaseObject::addMethod(const QString & strMethodName, const M & methodCallback)
{
	// create input arguments
	UA_Argument * p_inputArguments = nullptr;
	QVector<UA_Argument> listInputArguments;
	if (QOpcUaMethodTraits<M>::getNumArgs() > 0)
	{
		listInputArguments = QOpcUaMethodTraits<M>::getArgsUaArguments();
		p_inputArguments = listInputArguments.data();
	}
	// create output arguments
	UA_Argument * p_outputArgument = nullptr;
	UA_Argument outputArgument;
	if (!QOpcUaMethodTraits<M>::isRetUaArgumentVoid())
	{
		outputArgument = QOpcUaMethodTraits<M>::getRetUaArgument();
		p_outputArgument = &outputArgument;
	}
	// add method node
	QByteArray byteMethodName = strMethodName.toUtf8();
    UA_NodeId methNodeId = this->addMethodNodeInternal(
		byteMethodName, 
		QOpcUaMethodTraits<M>::getNumArgs(), 
		p_inputArguments, 
		p_outputArgument
	);
	// store method with node id hash as key
	Q_ASSERT_X(!m_hashMethods.contains(methNodeId), "QUaBaseObject::addMethodInternal", "Method already exists, callback will be overwritten.");
	m_hashMethods[methNodeId] = [methodCallback](const UA_Variant * input, UA_Variant * output) {
        // call method
		if (QOpcUaMethodTraits<M>::isRetUaArgumentVoid())
		{
			QOpcUaMethodTraits<M>::execCallback(methodCallback, input);
		}
		else
		{
			*output = QOpcUaMethodTraits<M>::execCallback(methodCallback, input);
		}
		// return success status
		return UA_STATUSCODE_GOOD;
	};
}

#endif // QUABASEOBJECT_H

