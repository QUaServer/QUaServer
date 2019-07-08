#ifndef QUASERVER_H
#define QUASERVER_H

#include <type_traits>

#include <QTimer>

#include <QUaTypesConverter>
#include <QUaFolderObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>
#include <QUaBaseObject>
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
#include <QUaBaseEvent>
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

// Enum Stuff
typedef qint64 QUaEnumKey;
struct QUaEnumEntry
{
	QByteArray strDisplayName;
	QByteArray strDescription;
};
Q_DECLARE_METATYPE(QUaEnumEntry);
inline bool operator==(const QUaEnumEntry& lhs, const QUaEnumEntry& rhs) 
{ 
	return lhs.strDisplayName == rhs.strDisplayName && lhs.strDescription == rhs.strDescription;
}
typedef QMap<QUaEnumKey, QUaEnumEntry> QUaEnumMap;
typedef QMapIterator<QUaEnumKey, QUaEnumEntry> QUaEnumMapIter;

// User validation
typedef std::function<bool(const QString &, const QString &)> QUaValidationCallback;

// Class whose only pupose is emit signals
class QUaSignaler : public QObject
{
	Q_OBJECT
public:
	explicit QUaSignaler(QObject *parent = 0) : QObject(parent) { };
signals:
	void signalNewInstance(QUaNode *node);
};

class QUaServer : public QObject
{
	friend class QUaNode;
	friend class QUaBaseVariable;
	friend class QUaBaseObject;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	friend class QUaBaseEvent;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	template <typename ClassType, typename R, bool IsMutable, typename... Args> friend struct QUaMethodTraitsBase;

	Q_OBJECT

public:

#ifndef UA_ENABLE_ENCRYPTION
	explicit QUaServer(const quint16    &intPort         = 4840, 
		               const QByteArray &byteCertificate = QByteArray(), 
		               QObject          *parent          = 0);
#else
	explicit QUaServer(const quint16    &intPort         = 4840, 
		               const QByteArray &byteCertificate = QByteArray(), 
		               const QByteArray &bytePrivateKey  = QByteArray(), 
		               QObject          *parent          = 0);
#endif
	
	~QUaServer();

	quint16 port() const;

	// Server Description API

	QString applicationName() const;
	void    setApplicationName(const QString &strApplicationName);
	QString applicationUri() const;
	void    setApplicationUri(const QString &strApplicationUri);
	QString productName() const;
	void    setProductName(const QString &strProductName);
	QString productUri() const;
	void    setProductUri(const QString &strProductUri);
	QString manufacturerName() const;
	void    setManufacturerName(const QString &strManufacturerName);
	QString softwareVersion() const;
	void    setSoftwareVersion(const QString &strSoftwareVersion);
	QString buildNumber() const;
	void    setBuildNumber(const QString &strBuildNumber);

	// Server LifeCycle API

	void start();
	void stop();
	bool isRunning() const;

	// Server Limits API

	quint16 maxSecureChannels() const;
	void    setMaxSecureChannels(const quint16 &maxSecureChannels);

	quint16 maxSessions() const;
	void    setMaxSessions(const quint16 &maxSessions);

	// Instance Creation API

	// register type in order to assign it a typeNodeId
	template<typename T>
	void registerType(const QString &strNodeId = "");
	// get all instances of a type
	template<typename T>
	QList<T*> typeInstances();
	// subscribe to instance of a type added
	template<typename T, typename M>
	QMetaObject::Connection instanceCreated(const M &callback);
	// same but pass a QObject pointer to disconnect callback when the QObject is deleted
	template<typename T, typename M>
	QMetaObject::Connection instanceCreated(const QObject * pObj, const M &callback);
	// same but pass a QObject pointer and member function as a callback
	template<typename T, typename TFunc1>
	QMetaObject::Connection instanceCreated(typename QtPrivate::FunctionPointer<TFunc1>::Object* pObj,
		                                    TFunc1 callback);
	// register enum in order to use it as data type
	template<typename T>
	void registerEnum(const QString &strNodeId = "");
	void registerEnum(const QString &strEnumName, const QUaEnumMap &mapEnum, const QString &strNodeId = "");
	// enum helpers
	bool isEnumRegistered(const QString &strEnumName);
	QUaEnumMap enumMap(const QString &strEnumName);
	void updateEnumEntry(const QString &strEnumName, const QUaEnumKey &enumValue, const QUaEnumEntry &enumEntry);
	void removeEnumEntry(const QString &strEnumName, const QUaEnumKey &enumValue);
	// register reference to get a respective refTypeId
	void registerReference(const QUaReference &ref);

	// create instance of a given (variable or object) type
	template<typename T>
	T* createInstance(QUaNode * parentNode, const QString &strNodeId = "");
	// get objects folder
	QUaFolderObject * objectsFolder();
	// get node reference by node id and cast to type (nullptr if node id does not exist)
	template<typename T>
	T* nodeById(const QString &strNodeId);
	// get node reference by node id (nullptr if node id does not exist)
	QUaNode * nodeById(const QString &strNodeId);

	// Browse API
	// (* actually browses using QObject tree)

	template<typename T>
	T* browsePath(const QStringList &strBrowsePath);
	// specialization
	QUaNode * browsePath(const QStringList &strBrowsePath);

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// Events API

	// create instance of a given event type
	template<typename T>
	T* createEvent();

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// Access Control API

	// anonymous login is enabled by default
	bool        anonymousLoginAllowed() const;
	void        setAnonymousLoginAllowed(const bool &anonymousLoginAllowed) const;
	// if user already exists, it updates password
	void        addUser(const QString &strUserName, const QString & strKey);
	// if user does not exist, it does nothing
	void        removeUser(const QString &strUserName);
	// get the key associated to the user
	QString     userKey(const QString &strUserName) const;
	// number of users
	int         userCount();
	// get all user names
	QStringList userNames() const;
	// check if user already exists
	bool        userExists(const QString &strUserName) const;
	// add a validation callback for user key, defaults checks key == password
	template<typename M>
	void        setUserValidationCallback(const M &callback);

signals:
	void iterateServer();

public slots:
	

private:
	UA_Server             * m_server;
	quint16                 m_port;
	UA_Boolean              m_running;
	QTimer                  m_iterWaitTimer;
	QByteArray              m_byteCertificate; // NOTE : needs to exists as long as server instance
	QUaFolderObject       * m_pobjectsFolder;

#ifdef UA_ENABLE_ENCRYPTION
	QByteArray              m_bytePrivateKey; // NOTE : needs to exists as long as server instance
#endif

	QByteArray m_byteApplicationName;
	QByteArray m_byteApplicationUri;

	QByteArray m_byteProductName;
	QByteArray m_byteProductUri;
	QByteArray m_byteManufacturerName;
	QByteArray m_byteSoftwareVersion;
	QByteArray m_byteBuildNumber;

	QHash<QString     , QString  > m_hashUsers;
	QHash<UA_NodeId   , QString  > m_hashSessions;
	QMap <QString     , UA_NodeId> m_mapTypes;
	QHash<QString     , UA_NodeId> m_hashEnums;
	QHash<QUaReference, UA_NodeId> m_hashRefs;
	QHash<UA_NodeId, QUaSignaler*> m_hashSignalers;
	QUaValidationCallback          m_validationCallback;

	// only call once on constructor
	static UA_ByteString * parseCertificate(const QByteArray &inByteCert, 
		                                    UA_ByteString    &outUaCert, 
		                                    QByteArray       &outByteCert);
	void setupServer();
	// types
	void registerType(const QMetaObject &metaObject, const QString &strNodeId = "");
	QList<QUaNode*> typeInstances(const QMetaObject &metaObject);
	template<typename T, typename M>
	QMetaObject::Connection instanceCreated(
		const QMetaObject &metaObject,
		const QObject * targetObject,
		const M &callback
	);
	// enums
	void       registerEnum(const QMetaEnum &metaEnum, const QString &strNodeId = "");
	UA_NodeId  enumValuesNodeId(const UA_NodeId &enumNodeId);
	UA_Variant enumValues(const UA_NodeId &enumNodeId);
	void       updateEnum(const UA_NodeId &enumNodeId, const QUaEnumMap &mapEnum);
	// lifecycle
    void registerTypeLifeCycle(const UA_NodeId &typeNodeId, const QMetaObject &metaObject);
	void registerTypeLifeCycle(const UA_NodeId *typeNodeId, const QMetaObject &metaObject);
	// meta
	void registerMetaEnums(const QMetaObject &parentMetaObject);
	void addMetaProperties(const QMetaObject &parentMetaObject);
	void addMetaMethods   (const QMetaObject &parentMetaObject);

	UA_NodeId createInstance(const QMetaObject &metaObject, QUaNode * parentNode, const QString &strNodeId = "");

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// create instance of a given event type
	UA_NodeId createEvent(const QMetaObject &metaObject, const UA_NodeId &nodeIdOriginator);

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	void bindCppInstanceWithUaNode(QUaNode * nodeInstance, UA_NodeId &nodeId);

	QHash< UA_NodeId, std::function<UA_StatusCode(const UA_NodeId *nodeId, void ** nodeContext)>> m_hashConstructors;
	QHash< UA_NodeId, std::function<UA_StatusCode(void *, const UA_Variant*, UA_Variant*)>      > m_hashMethods;

	static UA_NodeId getReferenceTypeId(const QMetaObject &parentMetaObject, const QMetaObject &childMetaObject);

	static UA_StatusCode uaConstructor(UA_Server        *server,
		                               const UA_NodeId  *sessionId, 
		                               void             *sessionContext,
		                               const UA_NodeId  *typeNodeId, 
		                               void             *typeNodeContext,
		                               const UA_NodeId  *nodeId, 
		                               void            **nodeContext);

	static void uaDestructor         (UA_Server        *server,
		                              const UA_NodeId  *sessionId, 
		                              void             *sessionContext,
		                              const UA_NodeId  *typeNodeId, 
		                              void             *typeNodeContext,
		                              const UA_NodeId  *nodeId, 
		                              void            **nodeContext);

	static UA_StatusCode uaConstructor(QUaServer         *server,
		                               const UA_NodeId   *nodeId, 
		                               void             **nodeContext,
		                               const QMetaObject &metaObject);

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

	static bool isNodeBound(const UA_NodeId &nodeId, UA_Server *server);

	struct QOpcUaEnumValue
	{
		UA_Int64         Value;
		UA_LocalizedText DisplayName;
		UA_LocalizedText Description;
	};

	static QUaServer * getServerNodeContext(UA_Server * server);

	static UA_StatusCode addEnumValues(UA_Server * server, UA_NodeId * parent, const UA_UInt32 numEnumValues, const QOpcUaEnumValue * enumValues);

	static UA_StatusCode activateSession(UA_Server                    *server, 
		                                 UA_AccessControl             *ac,
		                                 const UA_EndpointDescription *endpointDescription,
		                                 const UA_ByteString          *secureChannelRemoteCertificate,
		                                 const UA_NodeId              *sessionId,
		                                 const UA_ExtensionObject     *userIdentityToken,
		                                 void                        **sessionContext);

	static void closeSession(UA_Server        *server, 
		                     UA_AccessControl *ac, 
		                     const UA_NodeId  *sessionId, 
		                     void             *sessionContext);

	static UA_UInt32 getUserRightsMask(UA_Server        *server,
		                               UA_AccessControl *ac,
		                               const UA_NodeId  *sessionId,
		                               void             *sessionContext,
		                               const UA_NodeId  *nodeId,
		                               void             *nodeContext);

	static UA_Byte getUserAccessLevel(UA_Server        *server, 
		                              UA_AccessControl *ac,
		                              const UA_NodeId  *sessionId, 
		                              void             *sessionContext,
		                              const UA_NodeId  *nodeId, 
		                              void             *nodeContext);

	static UA_Boolean getUserExecutable(UA_Server        *server, 
		                                UA_AccessControl *ac,
		                                const UA_NodeId  *sessionId, 
		                                void             *sessionContext,
		                                const UA_NodeId  *methodId, 
		                                void             *methodContext);

	static UA_Boolean getUserExecutableOnObject(UA_Server        *server, 
		                                        UA_AccessControl *ac,
		                                        const UA_NodeId  *sessionId, 
		                                        void             *sessionContext,
		                                        const UA_NodeId  *methodId, 
		                                        void             *methodContext,
		                                        const UA_NodeId  *objectId, 
		                                        void             *objectContext);

	// NOTE : temporary values needed to instantiate node, used to simplify user API
	//        passed-in in QUaServer::uaConstructor and used in QUaNode::QUaNode
	const UA_NodeId   * m_newNodeNodeId;
	const QMetaObject * m_newNodeMetaObject;
	//        passed-in in QUaServer::createEvent, QUaBaseObject::createEvent and used in QUaBaseEvent::QUaBaseEvent
	const UA_NodeId   * m_newEventOriginatorNodeId;
};

template<typename T>
inline void QUaServer::registerType(const QString &strNodeId/* = ""*/)
{
	// call internal method
	this->registerType(T::staticMetaObject, strNodeId);
}

template<typename T>
inline QList<T*> QUaServer::typeInstances()
{
	QList<T*> retList;
	auto nodeList = this->typeInstances(T::staticMetaObject);
	for (int i = 0; i < nodeList.count(); i++)
	{
		auto instance = dynamic_cast<T*>(nodeList.at(i));
		Q_CHECK_PTR(instance);
		retList << instance;
	}
	return retList;
}

template<typename T, typename M>
inline QMetaObject::Connection QUaServer::instanceCreated(const M & callback)
{
	return this->instanceCreated<T>(T::staticMetaObject, nullptr, callback);
}

template<typename T, typename M>
inline QMetaObject::Connection QUaServer::instanceCreated(const QObject * pObj, const M & callback)
{
	return this->instanceCreated<T>(T::staticMetaObject, pObj, callback);
}

template<typename T, typename TFunc1>
inline QMetaObject::Connection QUaServer::instanceCreated(
	typename QtPrivate::FunctionPointer<TFunc1>::Object * pObj, 
	TFunc1 callback)
{
	// create callback as std::function and bind
	std::function<void(T*)> f = std::bind(callback, pObj, std::placeholders::_1);
	return this->instanceCreated<T>(T::staticMetaObject, pObj, f);
}

template<typename T, typename M>
inline QMetaObject::Connection QUaServer::instanceCreated(
	const QMetaObject & metaObject, 
	const QObject * targetObject, 
	const M & callback)
{
	// check if OPC UA relevant
	if (!metaObject.inherits(&QUaNode::staticMetaObject))
	{
		Q_ASSERT_X(false, "QUaServer::instanceCreated", "Unsupported base class. It must derive from QUaNode");
		return QMetaObject::Connection();
	}
	// try to get typeNodeId, if null, then register it
	QString   strClassName = QString(metaObject.className());
	UA_NodeId typeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (UA_NodeId_isNull(&typeNodeId))
	{
		this->registerType(metaObject);
		typeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	}
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));
	// check if there is already a signaler
	// NOTE : one signaler per registered ua type
	auto signaler = m_hashSignalers.value(typeNodeId, nullptr);
	if (!signaler)
	{
		m_hashSignalers[typeNodeId] = new QUaSignaler(this);
		signaler = m_hashSignalers.value(typeNodeId, nullptr);
	}
	Q_CHECK_PTR(signaler);
	// connect to signal
	return QObject::connect(signaler, &QUaSignaler::signalNewInstance, targetObject ? targetObject : this,
	[callback](QUaNode * node) {
		auto specializedNode = dynamic_cast<T*>(node);
		Q_CHECK_PTR(specializedNode);
		callback(specializedNode);
	}, Qt::QueuedConnection);
}

template<typename T>
inline void QUaServer::registerEnum(const QString &strNodeId/* = ""*/)
{
	// call internal method
	this->registerEnum(QMetaEnum::fromType<T>(), strNodeId);
}

template<typename T>
inline T * QUaServer::createInstance(QUaNode * parentNode, const QString &strNodeId/* = ""*/)
{
	// instantiate first in OPC UA
	UA_NodeId newInstanceNodeId = this->createInstance(T::staticMetaObject, parentNode, strNodeId);
	if (UA_NodeId_isNull(&newInstanceNodeId))
	{
		return nullptr;
	}
	// get new c++ instance created in UA constructor
	auto tmp = QUaNode::getNodeContext(newInstanceNodeId, this);
	T * newInstance = dynamic_cast<T*>(tmp);
	Q_CHECK_PTR(newInstance);
	// return c++ instance
	return newInstance;
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

template<typename T>
inline T * QUaServer::createEvent()
{
	// instantiate first in OPC UA
	UA_NodeId newEventNodeId = this->createEvent(T::staticMetaObject, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER));
	if (UA_NodeId_isNull(&newEventNodeId))
	{
		return nullptr;
	}
	// get new c++ instance created in UA constructor
	auto tmp = QUaNode::getNodeContext(newEventNodeId, this);
	T * newEvent = dynamic_cast<T*>(tmp);
	Q_CHECK_PTR(newEvent);
	// return c++ event instance
	return newEvent;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

template<typename T>
inline T * QUaServer::nodeById(const QString &strNodeId)
{
	return dynamic_cast<T*>(this->nodeById(strNodeId));
}

template<typename T>
inline T * QUaServer::browsePath(const QStringList & strBrowsePath)
{
	return dynamic_cast<T*>(this->browsePath(strBrowsePath));
}

template<typename M>
inline void QUaServer::setUserValidationCallback(const M & callback)
{
	m_validationCallback = [callback](const QString &strUserName, const QString &strPassword) {
		return callback(strUserName, strPassword);
	};
}

// -------- OTHER TYPES --------------------------------------------------

template<typename T>
inline T * QUaBaseObject::addChild(const QString &strNodeId/* = ""*/)
{
    return m_qUaServer->createInstance<T>(this, strNodeId);
}

template<typename T>
inline T * QUaBaseDataVariable::addChild(const QString &strNodeId/* = ""*/)
{
    return m_qUaServer->createInstance<T>(this, strNodeId);
}

template<typename T>
inline void QUaBaseVariable::setDataTypeEnum()
{
	// register if not registered
	m_qUaServer->registerEnum<T>();
	this->setDataTypeEnum(QMetaEnum::fromType<T>());
}

// https://stackoverflow.com/questions/13919234/is-there-a-way-using-c-type-traits-to-check-if-a-type-is-a-template-and-any-pr
template <typename> 
struct is_template : std::false_type {};

template <template<typename> typename H, typename S>
struct is_template<H<S>> : std::true_type {};

template <typename T>
struct template_traits;

template <template<typename> typename H, typename S>
struct template_traits<H<S>>
{
	using inner_type = S;
};

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

	// https://stackoverflow.com/questions/6627651/enable-if-method-specialization
	template<typename T>
	inline static UA_Argument getTypeUaArgument(QUaServer * uaServer, const int &iArg = 0)
	{
		return getTypeUaArgumentInternalArray<T>(is_template<T>(), uaServer, iArg);
	}

	template<typename T>
	inline static UA_Argument getTypeUaArgumentInternalArray(std::false_type, QUaServer * uaServer, const int &iArg = 0)
	{
		return getTypeUaArgumentInternalEnum<T>(std::is_enum<T>(), uaServer, iArg);
	}

	template<typename T>
	inline static UA_Argument getTypeUaArgumentInternalArray(std::true_type, QUaServer * uaServer, const int &iArg = 0)
	{
		UA_Argument arg = getTypeUaArgumentInternalEnum<typename template_traits<T>::inner_type>
			(std::is_enum<typename template_traits<T>::inner_type>(), uaServer, iArg);
		arg.valueRank = UA_VALUERANK_ONE_DIMENSION;
		return arg;
	}

    template<typename T>
    inline static UA_Argument getTypeUaArgumentInternalEnum(std::false_type, QUaServer * uaServer, const int &iArg = 0)
    {
        Q_UNUSED(uaServer);
        UA_NodeId nodeId = QUaTypesConverter::uaTypeNodeIdFromCpp<T>();
        return getTypeUaArgumentInternal<T>(nodeId, iArg);
    }

    template<typename T>
    inline static UA_Argument getTypeUaArgumentInternalEnum(std::true_type, QUaServer * uaServer, const int &iArg = 0)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        // compose enum name
        #if QT_VERSION >= 0x051200
            QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
        #else
            QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.name());
        #endif
        // register if not exists
        if (!uaServer->m_hashEnums.contains(strEnumName))
        {
            uaServer->registerEnum(metaEnum);
        }
        Q_ASSERT(uaServer->m_hashEnums.contains(strEnumName));
        // pass in enum nodeid
        UA_NodeId nodeId = uaServer->m_hashEnums.value(strEnumName);
        return getTypeUaArgumentInternal<T>(nodeId, iArg);
    }

    template<typename T>
    inline static UA_Argument getTypeUaArgumentInternal(const UA_NodeId &nodeId, const int &iArg = 0)
    {
        UA_Argument inputArgument;
        UA_Argument_init(&inputArgument);
        // create n-th argument with name "Arg" + number
        inputArgument.description = UA_LOCALIZEDTEXT((char *)"", (char *)"Method Argument");
        inputArgument.name        = QUaTypesConverter::uaStringFromQString(QObject::trUtf8("Arg%1").arg(iArg));
        inputArgument.dataType    = nodeId;
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
		return getRetUaArgumentArray<R>(is_template<R>());
	}

	template<typename T>
    inline static UA_Argument getRetUaArgumentArray(std::false_type)
    {
        if (isRetUaArgumentVoid()) return UA_Argument();
        // create output argument
        UA_Argument outputArgument;
        UA_Argument_init(&outputArgument);
        outputArgument.description = UA_LOCALIZEDTEXT((char *)"",
                                                      (char *)"Result Value");
        outputArgument.name        = QUaTypesConverter::uaStringFromQString((char *)"Result");
        outputArgument.dataType    = QUaTypesConverter::uaTypeNodeIdFromCpp<T>();
        outputArgument.valueRank   = UA_VALUERANK_SCALAR;
        return outputArgument;
    }

	template<typename T>
	inline static UA_Argument getRetUaArgumentArray(std::true_type)
	{
		UA_Argument arg = getRetUaArgumentArray<typename template_traits<T>::inner_type>(is_template<typename template_traits<T>::inner_type>());
		arg.valueRank = UA_VALUERANK_ONE_DIMENSION;
		return arg;
	}

    inline static QVector<UA_Argument> getArgsUaArguments(QUaServer * uaServer)
    {
        int iArg = 0;
        const size_t nArgs = getNumArgs();
        if (nArgs <= 0) return QVector<UA_Argument>();
        return { getTypeUaArgument<Args>(uaServer, iArg++)... };
    }

    template<typename T>
    inline static T convertArgType(const UA_Variant * input, const int &iArg)
    {
		return convertArgTypeArray<T>(is_template<T>(), input, iArg);
    }

	template<typename T>
	inline static T convertArgTypeArray(std::false_type, const UA_Variant * input, const int &iArg)
	{
		QVariant varQt = QUaTypesConverter::uaVariantToQVariant(input[iArg]);
		return varQt.value<T>();
	}

	template<typename T>
	inline static T convertArgTypeArray(std::true_type, const UA_Variant * input, const int &iArg)
	{
		T retArr;
		auto varQt = QUaTypesConverter::uaVariantToQVariant(input[iArg]);
		auto iter  = varQt.value<QSequentialIterable>();
		for (const QVariant &v : iter)
		{
			retArr << v.value<template_traits<T>::inner_type>();
		}
		return retArr;
	}

    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
    {
        // call method
        // NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
        int iArg = (int)getNumArgs() - 1;
        // call method
        QVariant varResult = QVariant::fromValue(methodCallback(convertArgType<Args>(input, iArg--)...));
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

template<typename M>
inline void QUaBaseObject::addMethod(const QString & strMethodName, const M & methodCallback)
{
    // create input arguments
    UA_Argument * p_inputArguments = nullptr;
    QVector<UA_Argument> listInputArguments;
    if (QOpcUaMethodTraits<M>::getNumArgs() > 0)
    {
        listInputArguments = QOpcUaMethodTraits<M>::getArgsUaArguments(m_qUaServer);
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

#endif // QUASERVER_H
