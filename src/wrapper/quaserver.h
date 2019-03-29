#ifndef QUASERVER_H
#define QUASERVER_H

#include <type_traits>

#include <QUaTypesConverter>
#include <QUaFolderObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>
#include <QUaBaseObject>

class QUaServer : public QObject
{
	Q_OBJECT

friend class QUaNode;
friend class QUaBaseVariable;
friend class QUaBaseObject;

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

	void start();
	void stop();
	bool isRunning();

	// register type in order to assign it a typeNodeId
	template<typename T>
	void registerType();
	// register enum in order to use it as data type
	template<typename T>
	void registerEnum();
	// register reference to get a respective refTypeId
	void registerReference(const QUaReference &ref);

	// create instance of a given type
	template<typename T>
	T* createInstance(QUaNode * parentNode, const QString &strNodeId = "");
	// get objects folder
	QUaFolderObject * objectsFolder();
	// get node reference by node id and cast to type (nullptr if node id does not exist)
	template<typename T>
	T* getNodebyId(const QString &strNodeId);
	// get node reference by node id (nullptr if node id does not exist)
	QUaNode * getNodebyId(const QString &strNodeId);

signals:
	void iterateServer();

public slots:
	

private:
	UA_Server             * m_server;
	UA_ServerConfig       * m_config;
	UA_Boolean              m_running;
	QByteArray              m_byteCertificate; // NOTE : needs to exists as long as server instance
	QMetaObject::Connection m_connection;
	QUaFolderObject       * m_pobjectsFolder;

#ifdef UA_ENABLE_ENCRYPTION
	QByteArray              m_bytePrivateKey; // NOTE : needs to exists as long as server instance
#endif

	QMap <QString     , UA_NodeId> m_mapTypes;
	QHash<QString     , UA_NodeId> m_hashEnums;
	QHash<QUaReference, UA_NodeId> m_hashRefs;

	// only call once on constructor
	UA_ByteString * parseCertificate(const QByteArray &inByteCert, 
		                             UA_ByteString    &outUaCert, 
		                             QByteArray       &outByteCert);
	void setupServer();

	void registerType(const QMetaObject &metaObject);
	void registerEnum(const QMetaEnum &metaEnum);

    void registerTypeLifeCycle(const UA_NodeId &typeNodeId, const QMetaObject &metaObject);
	void registerTypeLifeCycle(const UA_NodeId *typeNodeId, const QMetaObject &metaObject);

	void registerMetaEnums(const QMetaObject &parentMetaObject);
	void addMetaProperties(const QMetaObject &parentMetaObject);
	void addMetaMethods   (const QMetaObject &parentMetaObject);

	UA_NodeId createInstance(const QMetaObject &metaObject, QUaNode * parentNode, const QString &strNodeId = "");

	void bindCppInstanceWithUaNode(QUaNode * nodeInstance, UA_NodeId &nodeId);

	QHash< UA_NodeId, std::function<UA_StatusCode(const UA_NodeId *nodeId, void ** nodeContext)>> m_hashConstructors;
	QHash< UA_NodeId, QList<std::function<void(void)>>                                          > m_hashDeferredConstructors;
	QHash< UA_NodeId, std::function<UA_StatusCode(void *, const UA_Variant*, UA_Variant*)>      > m_hashMethods;

	static UA_NodeId getReferenceTypeId(const QString &strParentClassName, const QString &strChildClassName);

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

	static UA_StatusCode createEnumValue(const QOpcUaEnumValue * enumVal, UA_ExtensionObject * outExtObj);

	static UA_StatusCode addEnumValues(UA_Server * server, UA_NodeId * parent, const UA_UInt32 numEnumValues, const QOpcUaEnumValue * enumValues);

	// NOTE : temporary values needed to instantiate node, used to simplify user API
	//        passed-in in QUaServer::uaConstructor and used in QUaNode::QUaNode
	const UA_NodeId   * m_newNodeNodeId;
	const QMetaObject * m_newNodeMetaObject;

};

template<typename T>
inline void QUaServer::registerType()
{
	// call internal method
	this->registerType(T::staticMetaObject);
}

template<typename T>
inline void QUaServer::registerEnum()
{
	// call internal method
	this->registerEnum(QMetaEnum::fromType<T>());
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

template<typename T>
inline T * QUaServer::getNodebyId(const QString &strNodeId)
{
	return dynamic_cast<T*>(this->getNodebyId(strNodeId));
}

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

#endif // QUASERVER_H
