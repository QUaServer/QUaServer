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

	static UA_NodeId getReferenceTypeId(const QString &strParentClassName, const QString &strChildClassName);
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
	Q_ASSERT(!UA_NodeId_isNull(&parentNode->m_nodeId));
	// try to get typeNodeId, if null, then register it
	QString   strClassName = QString(T::staticMetaObject.className());
	UA_NodeId typeNodeId   = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	if (UA_NodeId_isNull(&typeNodeId))
	{
		this->registerType<T>();
		typeNodeId = m_mapTypes.value(strClassName, UA_NODEID_NULL);
	}
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));
	// instantiate C++ object or variable
	T *  childNode = new T(parentNode);

	// adapt parent relation with child according to parent type
	UA_NodeId referenceTypeId = QOpcUaServer::getReferenceTypeId(parentNode->metaObject()->className(), 
		                                                         childNode->metaObject()->className());

	// set qualified name, default is class name
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name           = QOpcUaTypesConverter::uaStringFromQString("");

	// check if object or variable
	// NOTE : a type is considered to inherit itself (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
	if (T::staticMetaObject.inherits(&QOpcUaBaseObject::staticMetaObject))
	{
		UA_ObjectAttributes oAttr     = UA_ObjectAttributes_default;
		// add object
		auto st = UA_Server_addObjectNode(m_server,
                                          UA_NODEID_NULL,       // requested nodeId
                                          parentNode->m_nodeId, // parent
                                          referenceTypeId,      // parent relation with child
                                          browseName,
                                          typeNodeId,
                                          oAttr, 
                                          (void*)childNode,      // set new instance as context
                                          &childNode->m_nodeId); // set new nodeId to new instance
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	else if (T::staticMetaObject.inherits(&QOpcUaBaseVariable::staticMetaObject))
	{
		UA_VariableAttributes vAttr   = UA_VariableAttributes_default;

	  
		

		// add variable
		auto st = UA_Server_addVariableNode(m_server,
                                            UA_NODEID_NULL,       // requested nodeId
                                            parentNode->m_nodeId, // parent
                                            referenceTypeId,      // parent relation with child
                                            browseName,
                                            typeNodeId, 
                                            vAttr, 
                                            (void*)childNode,      // set new instance as context
                                            &childNode->m_nodeId); // set new nodeId to new instance
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
	}
	else
	{
		Q_ASSERT_X(false, "QOpcUaServer::createInstance", "Unsopported type.");
		delete childNode;
		return nullptr;
	}

	return childNode;
}


#endif // QOPCUASERVER_H
