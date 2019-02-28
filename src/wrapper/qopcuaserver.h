#ifndef QOPCUASERVER_H
#define QOPCUASERVER_H

#include <type_traits>

#include <QOpcUaTypesConverter>
#include <QOpcUaFolderObject>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaProperty>

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

	QMap<QString, UA_NodeId> m_mapTypes;

	void registerType(const QMetaObject &metaObject);

	void addMetaProperties(const QMetaObject &parentMetaObject);

	UA_NodeId createInstance(const QMetaObject &metaObject, QOpcUaServerNode * parentNode);

	void bindCppInstanceWithUaNode(QOpcUaServerNode * nodeInstance, UA_NodeId &nodeId);

	QHash< UA_NodeId, std::function<UA_StatusCode(UA_Server       *server,
                                                  const UA_NodeId *sessionId, 
                                                  void            *sessionContext,
                                                  const UA_NodeId *typeNodeId, 
                                                  void            *typeNodeContext,
                                                  const UA_NodeId *nodeId, 
                                                  void           **nodeContext)>> m_hashConstructors;

	static UA_NodeId getReferenceTypeId(const QString &strParentClassName, const QString &strChildClassName);

	static UA_StatusCode uaConstructor(UA_Server        *server,
		                               const UA_NodeId  *sessionId, 
		                               void             *sessionContext,
		                               const UA_NodeId  *typeNodeId, 
		                               void             *typeNodeContext,
		                               const UA_NodeId  *nodeId, 
		                               void            **nodeContext);
};

template<typename T>
inline void QOpcUaServer::registerType()
{
	// call internal method
	this->registerType(T::staticMetaObject);
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

	// TODO : find a way to instantiate in UA constructor and then get the c++ ref with
	//        UA_Server_getNodeContext

	// create new c++ instance
	T * childNode = new T(parentNode);
	// bind
	this->bindCppInstanceWithUaNode(childNode, newInstanceNodeId);
	// return c++ instance
	return childNode;
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

#endif // QOPCUASERVER_H
