#ifndef QOPCUASERVER_H
#define QOPCUASERVER_H

#include <type_traits>

#include <QOpcUaTypesConverter>
#include <QOpcUaFolderObject>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaProperty>
#include "qopcuabaseobject.h"

class QOpcUaServer : public QObject
{
	Q_OBJECT

friend class QOpcUaServerNode;
friend class QOpcUaBaseVariable;
friend class QOpcUaBaseObject;
// NOTE : this is how we would declare a <template class> friend
//template<typename T> friend class QOpcUaServerNodeFactory;

public:
    explicit QOpcUaServer(QObject *parent = 0);

	void start();
	void stop();
	bool isRunning();

	// register type in order to assign it a typeNodeId
	template<typename T>
	void registerType();

	// register enum in order to use it as data type
	template<typename T>
	void registerEnum();

	// create instance of a given type
	template<typename T>
	T* createInstance(QOpcUaServerNode * parentNode);

	QOpcUaFolderObject * get_objectsFolder();

signals:
	void iterateServer();

public slots:


private:
	UA_Server * m_server;
	UA_Boolean  m_running;
	QMetaObject::Connection m_connection;

	QOpcUaFolderObject * m_pobjectsFolder;

	QMap <QString, UA_NodeId> m_mapTypes;
	QHash<QString, UA_NodeId> m_hashEnums;

	void registerType(const QMetaObject &metaObject);
	void registerEnum(const QMetaEnum &metaEnum);

    void registerTypeLifeCycle(const UA_NodeId &typeNodeId, const QMetaObject &metaObject);
	void registerTypeLifeCycle(const UA_NodeId *typeNodeId, const QMetaObject &metaObject);

	void registerMetaEnums(const QMetaObject &parentMetaObject);
	void addMetaProperties(const QMetaObject &parentMetaObject);
	void addMetaMethods   (const QMetaObject &parentMetaObject);

	UA_NodeId createInstance(const QMetaObject &metaObject, QOpcUaServerNode * parentNode);

	void bindCppInstanceWithUaNode(QOpcUaServerNode * nodeInstance, UA_NodeId &nodeId);

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

	static UA_StatusCode uaConstructor(QOpcUaServer      *server,
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

};

template<typename T>
inline void QOpcUaServer::registerType()
{
	// call internal method
	this->registerType(T::staticMetaObject);
}

template<typename T>
inline void QOpcUaServer::registerEnum()
{
	// call internal method
	this->registerEnum(QMetaEnum::fromType<T>());
}

template<typename T>
inline T * QOpcUaServer::createInstance(QOpcUaServerNode * parentNode)
{
	// instantiate first in OPC UA
	UA_NodeId newInstanceNodeId = this->createInstance(T::staticMetaObject, parentNode);
	if (newInstanceNodeId == UA_NODEID_NULL)
	{
		return nullptr;
	}
	// get new c++ instance created in UA constructor
	auto tmp = QOpcUaServerNode::getNodeContext(newInstanceNodeId, this);
	T * newInstance = qobject_cast<T*>(tmp);
	Q_CHECK_PTR(newInstance);
	// return c++ instance
	return newInstance;
}

template<typename T>
inline T * QOpcUaBaseObject::addChild()
{
    return m_qopcuaserver->createInstance<T>(this);
}

template<typename T>
inline T * QOpcUaBaseDataVariable::addChild()
{
    return m_qopcuaserver->createInstance<T>(this);
}

template<typename T>
inline void QOpcUaBaseVariable::setDataTypeEnum()
{
	// register if not registered
	m_qopcuaserver->registerEnum<T>();
	this->setDataTypeEnum(QMetaEnum::fromType<T>());
}

#endif // QOPCUASERVER_H
