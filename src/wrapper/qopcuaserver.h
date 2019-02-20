#ifndef QOPCUASERVER_H
#define QOPCUASERVER_H

#include <QOpcUaTypesConverter>
#include <type_traits>

class QOpcUaFolderObject;

class QOpcUaServer : public QObject
{
	Q_OBJECT

friend class QOpcUaServerNode;
friend class QOpcUaBaseVariable;

public:
    explicit QOpcUaServer(QObject *parent = 0);

	void start();

	// register type in order to assign it a typeNodeId
	template<typename T>
	void registerType();

	// create instance of a given type
	template<typename T>
	T* createInstance(QOpcUaServerNode * parentNode);

	QOpcUaFolderObject * get_objectsFolder();
	

signals:

public slots:

private:
	UA_Server * m_server;

	QOpcUaFolderObject * m_pobjectsFolder;
};

template<typename T>
inline void QOpcUaServer::registerType()
{
	// TODO : T::SetTypeNodeId( ... );

	QOpcUaNodeFactory<T>::SetTypeNodeId(UA_NODEID_NULL);

	/*
	if (T::staticMetaObject.inherits(&QOpcUaAbstractObject::staticMetaObject))
	{
		UA_Server_addObjectTypeNode
	}
	else if (T::staticMetaObject.inherits(&QOpcUaAbstractVariable::staticMetaObject))
	{
		UA_Server_addVariableTypeNode
	}
	*/
}

template<typename T>
inline T * QOpcUaServer::createInstance(QOpcUaServerNode * parentNode)
{
	Q_ASSERT(!UA_NodeId_isNull(&parentNode->m_nodeId));
	// try to get typeNodeId, if null, then register it
	UA_NodeId typeNodeId = QOpcUaNodeFactory<T>::GetTypeNodeId();
	if (UA_NodeId_isNull(&typeNodeId))
	{
		this->registerType<T>();
		typeNodeId = QOpcUaNodeFactory<T>::GetTypeNodeId();
	}
	Q_ASSERT(!UA_NodeId_isNull(&typeNodeId));
	// instantiate C++ object or variable
	T *  instance = new T(parentNode);

	// TODO : adapt parent relation (referenceTypeId) to parent type

	// check if object or variable
	// NOTE : a type is considered to inherit itself (http://doc.qt.io/qt-5/qmetaobject.html#inherits)
	if (T::staticMetaObject.inherits(&QOpcUaBaseObject::staticMetaObject))
	{
		// add to OpcUa
		UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		UA_QualifiedName    bName = UA_QUALIFIEDNAME_ALLOC(1, T::staticMetaObject.className()); // TODO : get from instance? pass as argument?
		UA_Server_addObjectNode(m_server, 
			                    UA_NODEID_NULL,
			                    parentNode->m_nodeId,                     // parent
			                    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), // parent relation with child
			                    bName,
			                    typeNodeId,
			                    oAttr, 
			                    (void*)instance,      // set new instance as context
			                    &instance->m_nodeId); // set new nodeId to new instance
		// TODO ?
		// UA_QualifiedName_deleteMembers or UA_QualifiedName_delete ??
	}
	else if (T::staticMetaObject.inherits(&QOpcUaBaseVariable::staticMetaObject))
	{
		UA_VariableAttributes vAttr = UA_VariableAttributes_default;
		UA_QualifiedName      qName = UA_QUALIFIEDNAME_ALLOC(1, T::staticMetaObject.className()); // TODO : get from instance? pass as argument?
		UA_Server_addVariableNode(m_server,
			                      UA_NODEID_NULL, 
			                      parentNode->m_nodeId,                        // parent
			                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), // parent relation with child
			                      qName,
			                      typeNodeId, 
			                      vAttr, 
			                      (void*)instance,      // set new instance as context
			                      &instance->m_nodeId); // set new nodeId to new instance
	}
	else
	{
		Q_ASSERT_X(false, "QOpcUaServer::createInstance", "Unsopported type.");
		delete instance;
		return nullptr;
	}

	return instance;
}


#endif // QOPCUASERVER_H
