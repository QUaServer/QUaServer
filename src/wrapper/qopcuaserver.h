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

	static UA_NodeId getReferenceTypeId(const QString &strParentClassName, const QString &strChildClassName);
};

template<typename T>
inline void QOpcUaServer::registerType()
{
	// check if already register
	QString   strClassName  = QString(T::staticMetaObject.className());
	UA_NodeId newTypeNodeId = QOpcUaNodeFactory<T>::GetTypeNodeId();
	if (!UA_NodeId_isNull(&newTypeNodeId))
	{
		// add to map of not here yet
		if (!m_mapTypes.contains(strClassName))
		{
			m_mapTypes[strClassName] = newTypeNodeId;
		}
		return;
	}
	// else register
	if (T::staticMetaObject.inherits(&QOpcUaBaseObject::staticMetaObject))
	{
		// create new object type browse name
		UA_QualifiedName browseName;
		browseName.namespaceIndex = 1;
		browseName.name = QOpcUaTypesConverter::uaStringFromQString(QOpcUaObjectTypeTraits<T>::GetBrowseName());
		// create object type attributes
		UA_ObjectTypeAttributes otAttr = UA_ObjectTypeAttributes_default;
		// set node attributes		  
		QByteArray byteDisplayName     = QOpcUaObjectTypeTraits<T>::GetDisplayName().toUtf8();
		otAttr.displayName             = UA_LOCALIZEDTEXT((char*)"en-US", byteDisplayName.data());
		QByteArray byteDescription     = QOpcUaObjectTypeTraits<T>::GetDescription().toUtf8();
		otAttr.description             = UA_LOCALIZEDTEXT((char*)"en-US", byteDescription.data());
		otAttr.writeMask               = QOpcUaObjectTypeTraits<T>::GetWriteMask();
		// set object type attributes  
		otAttr.isAbstract              = QOpcUaObjectTypeTraits<T>::GetIsAbstract();
		// add new object type
		QString strBaseClassName       = QString(T::staticMetaObject.superClass()->className());
		Q_ASSERT_X(m_mapTypes.contains(strBaseClassName), "QOpcUaServer::registerType", "Base object type not registered.");
		auto st = UA_Server_addObjectTypeNode(m_server,
			                                  UA_NODEID_NULL,                            // requested nodeId
			                                  m_mapTypes[strBaseClassName],              // parent (type)
			                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), // parent relation with child
			                                  browseName,
			                                  otAttr,
			                                  nullptr,                                   // no context ?
			                                  &newTypeNodeId);                           // new object type id
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		// set type node id in traits
		QOpcUaNodeFactory<T>::SetTypeNodeId(newTypeNodeId);
	}
	else if (T::staticMetaObject.inherits(&QOpcUaBaseDataVariable::staticMetaObject))
	{
		// create new variable type browse name
		UA_QualifiedName browseName;
		browseName.namespaceIndex = 1;
		browseName.name = QOpcUaTypesConverter::uaStringFromQString(QOpcUaVariableTypeTraits<T>::GetBrowseName());
		// create variable type attributes
		UA_VariableTypeAttributes vtAttr = UA_VariableTypeAttributes_default;
		// set node attributes		  
		QByteArray byteDisplayName       = QOpcUaVariableTypeTraits<T>::GetDisplayName().toUtf8();
		vtAttr.displayName               = UA_LOCALIZEDTEXT((char*)"en-US", byteDisplayName.data());
		QByteArray byteDescription       = QOpcUaVariableTypeTraits<T>::GetDescription().toUtf8();
		vtAttr.description               = UA_LOCALIZEDTEXT((char*)"en-US", byteDescription.data());
		vtAttr.writeMask                 = QOpcUaVariableTypeTraits<T>::GetWriteMask();
		// set variable type attributes  
		QVariant varDefaultValue         = QOpcUaVariableTypeTraits<T>::GetDefaultValue();
		vtAttr.value                     = QOpcUaTypesConverter::uaVariantFromQVariant(varDefaultValue);
		vtAttr.dataType                  = QOpcUaTypesConverter::uaTypeNodeIdFromQType((QMetaType::Type)varDefaultValue.type());
		vtAttr.valueRank                 = QOpcUaBaseVariable::GetValueRankFromQVariant(varDefaultValue);
		QVector<quint32> qtArrayDim      = QOpcUaBaseVariable::GetArrayDimensionsFromQVariant(varDefaultValue);
		vtAttr.arrayDimensions           = qtArrayDim.count() > 0 ? qtArrayDim.data() : nullptr;
		vtAttr.arrayDimensionsSize       = qtArrayDim.count();
		vtAttr.isAbstract                = QOpcUaVariableTypeTraits<T>::GetIsAbstract();
		// add new variable type
		QString strBaseClassName         = QString(T::staticMetaObject.superClass()->className());
		Q_ASSERT_X(m_mapTypes.contains(strBaseClassName), "QOpcUaServer::registerType", "Base variable type not registered.");
		auto st = UA_Server_addVariableTypeNode(m_server,
			                                    UA_NODEID_NULL,                            // requested nodeId
			                                    m_mapTypes[strBaseClassName],              // parent (type)
			                                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), // parent relation with child
			                                    browseName,
			                                    UA_NODEID_NULL,                            // typeDefinition ??
			                                    vtAttr,
			                                    nullptr,                                   // no context ?
			                                    &newTypeNodeId);                           // new variable type id
		Q_ASSERT(st == UA_STATUSCODE_GOOD);
		Q_UNUSED(st);
		// set type node id in traits
		QOpcUaNodeFactory<T>::SetTypeNodeId(newTypeNodeId);
	}
	else
	{
		Q_ASSERT_X(false, "QOpcUaServer::registerType", "Unsupported base class");
	}
	// add to map of not here yet
	if (!m_mapTypes.contains(strClassName))
	{
		m_mapTypes[strClassName] = newTypeNodeId;
	}
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
