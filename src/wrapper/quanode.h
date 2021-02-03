#ifndef QUASERVERNODE_H
#define QUASERVERNODE_H

#include <functional>
#include <typeinfo>

#include <QObject>
#include <QVariant>
#include <QMetaProperty>
#include <QQueue>

#include <open62541.h>

class QUaServer;
class QUaProperty;
class QUaBaseDataVariable;
class QUaBaseObject;
class QUaFolderObject;
class QUaSession;

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
class QUaCondition;
class QUaConditionBranch;
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaTypesConverter>

// trait used to check if type has bool T::serializeStart(QQueue<QUaLog>&)
template <typename T, typename = void>
struct QUaHasMethodSerializeStart
	: std::false_type
{};

template <typename T>
struct QUaHasMethodSerializeStart<T,
	typename std::enable_if<std::is_same<decltype(&T::serializeStart), bool(T::*)(QQueue<QUaLog>&)>::value>::type>
	: std::true_type
{};

// trait used to check if type has bool T::serializeEnd(QQueue<QUaLog>&)
template <typename T, typename = void>
struct QUaHasMethodSerializeEnd
	: std::false_type
{};

template <typename T>
struct QUaHasMethodSerializeEnd<T,
	typename std::enable_if<std::is_same<decltype(&T::serializeEnd), bool(T::*)(QQueue<QUaLog>&)>::value>::type>
	: std::true_type
{};

// trait used to check if type has bool T::deserializeStart(QQueue<QUaLog>&)
template <typename T, typename = void>
struct QUaHasMethodDeserializeStart
	: std::false_type
{};

template <typename T>
struct QUaHasMethodDeserializeStart<T,
	typename std::enable_if<std::is_same<decltype(&T::deserializeStart), bool(T::*)(QQueue<QUaLog>&)>::value>::type>
	: std::true_type
{};

// trait used to check if type has bool T::deserializeEnd(QQueue<QUaLog>&)
template <typename T, typename = void>
struct QUaHasMethodDeserializeEnd
	: std::false_type
{};

template <typename T>
struct QUaHasMethodDeserializeEnd<T,
	typename std::enable_if<std::is_same<decltype(&T::deserializeEnd), bool(T::*)(QQueue<QUaLog>&)>::value>::type>
	: std::true_type
{};


class QUaNode : public QObject
{
	friend class QUaServer;
	friend class QUaBaseObject;
	friend class QUaBaseVariable;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	friend class QUaBaseEvent;
	friend class QUaServer_Anex;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
#ifdef UA_ENABLE_HISTORIZING
	friend class QUaHistoryBackend;
#endif // UA_ENABLE_HISTORIZING
#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	friend class QUaCondition;
	friend class QUaConditionBranch;
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
	Q_OBJECT

		// Node Attributes

		// N/A : not an OPC UA node attribute, is it a library helper?
		//Q_PROPERTY(quint32 specifiedAttributes READ get_specifiedAttributes)

		Q_PROPERTY(QUaLocalizedText displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
		Q_PROPERTY(QUaLocalizedText description READ description WRITE setDescription NOTIFY descriptionChanged)
		// Exposes the possibilities of a client to write the Attributes of the Node.
		Q_PROPERTY(quint32 writeMask   READ writeMask   WRITE setWriteMask   NOTIFY writeMaskChanged)

		// Cannot be read, since the local API user always has full rights.
		// Cannot be written from the server, as they are specific to the different users and set by the access control callback.
		// It is defined by overwriting the server's config->accessControl.getUserRightsMask (see getUserRightsMask_default)
		//Q_PROPERTY(quint32 userWriteMask READ get_userWriteMask)

		// Node Specifics

		// Cannot be changed once a node has been created.
		Q_PROPERTY(QUaNodeId nodeId     READ nodeId)
		Q_PROPERTY(QString   nodeClass  READ nodeClass)

		// Other Properties

		Q_PROPERTY(QUaQualifiedName browseName READ browseName)

public:
	explicit QUaNode(
		QUaServer* server
	);

	// Virtual destructor is necessary to call derived class destructor when delete is called on pointer to base class
	// this can be useful when the library must delete a node because it was requested through the network
	// in which case we only have available a pointer to the base class, mainly because we support the user deriving 
	// from the library classes and we want to also call their custom destructors.
	// https://stackoverflow.com/questions/294927/does-delete-work-with-pointers-to-base-class
	// https://repl.it/repls/EachSpicyInternalcommand
	virtual ~QUaNode();

	bool operator ==(const QUaNode& other) const;

	QUaServer* server() const;

	// Attributes API

	virtual QUaLocalizedText displayName() const;
	virtual void setDisplayName(const QUaLocalizedText& displayName);

	QUaLocalizedText description() const;
	void setDescription(const QUaLocalizedText& description);

	quint32 writeMask() const;
	void setWriteMask(const quint32& writeMask);

	QUaNodeId nodeId() const;
	QString nodeClass() const;

	QUaQualifiedName browseName() const;

	// Instance Creation API

	virtual QUaProperty * addProperty(
		const QUaQualifiedName& browseName,
		const QUaNodeId& nodeId = QUaNodeId()
	);
	virtual QUaBaseDataVariable* addBaseDataVariable(
		const QUaQualifiedName& browseName,
		const QUaNodeId& nodeId = QUaNodeId()
	);
	virtual QUaBaseObject* addBaseObject(
		const QUaQualifiedName& browseName,
		const QUaNodeId& nodeId = QUaNodeId()
	);
	virtual QUaFolderObject* addFolderObject(
		const QUaQualifiedName& browseName,
		const QUaNodeId& nodeId = QUaNodeId()
	);

	// Browse API
	// (* actually browses using QObject tree)

	QUaNodeId        typeDefinitionNodeId() const;
	QString          typeDefinitionDisplayName() const;
	QUaQualifiedName typeDefinitionBrowseName() const;

	// if browseName empty, get all children
	template<typename T>
	QList<T*> browseChildren() const;
	// specialization
	QList<QUaNode*> browseChildren() const;

	// just get the first one
	// if instantiateOptional then create optional child of instance declaration (if child exists in type)
	template<typename T>
	T* browseChild(const QUaQualifiedName& browseName, const bool& instantiateOptional = false);
	// specialization
	QUaNode* browseChild(const QUaQualifiedName& browseName, const bool& instantiateOptional = false);

	bool hasChild(const QUaQualifiedName& browseName);

	template<typename T>
	T* browsePath(const QUaBrowsePath& browsePath) const;
	// specialization
	QUaNode* browsePath(const QUaBrowsePath& browsePath) const;

	// get node's browse path starting from ObjectsFolder
	QUaBrowsePath nodeBrowsePath() const;

	// Reference API

	void addReference(const QUaReferenceType& refType, QUaNode* nodeTarget, const bool& isForward = true);

	void removeReference(const QUaReferenceType& refType, QUaNode* nodeTarget, const bool& isForward = true);

	template<typename T>
	QList<T*>       findReferences(const QUaReferenceType& refType, const bool& isForward = true) const;
	// specialization
	QList<QUaNode*> findReferences(const QUaReferenceType& refType, const bool& isForward = true) const;

	// Access Control API

	// Access Control for Attributes
	// TODO : not tested thoroughly

	// reimplement for custom access control for a given custom UA type (derived C++ class)
	virtual QUaWriteMask userWriteMask(const QString& strUserName);

	// provide specific implementation for individual nodes
	// signature is <QUaWriteMask(const QString &)>
	template<typename M>
	void setUserWriteMaskCallback(const M& callback);

	// Access Control for Variables

	// reimplement for custom access control for a given custom UA type (derived C++ class)
	virtual QUaAccessLevel userAccessLevel(const QString& strUserName);

	// provide specific implementation for individual variable nodes
	// signature is <QUaAccessLevel(const QString &)>
	template<typename M>
	void setUserAccessLevelCallback(const M& callback);

	// Access Control for Methods
	// TODO : not working properly until
	//        https://github.com/open62541/open62541/pull/1812 is fixed

	// reimplement for custom access control for a given custom UA type (derived C++ class)
	virtual bool userExecutable(const QString& strUserName);

	// provide specific implementation for individual object nodes
	// signature is <bool(const QString &)>
	template<typename M>
	void setUserExecutableCallback(const M& callback);

	// Serialization API

	QString className() const;

	// T must implement:
	// bool T::writeInstance(
	// 	const QUaNodeId& nodeId,
	// 	const QString& typeName,
	// 	const QMap<QString, QVariant>& attrs,
	// 	const QList<QUaForwardReference>& forwardRefs,
	// 	QQueue<QUaLog>& logOut
	// );
	// T can optionally implement:
	// bool T::serializeStart(QQueue<QUaLog>& logOut);
	// bool T::serializeEnd(QQueue<QUaLog>& logOut);
	template<typename T>
	bool serialize(T& serializer, QQueue<QUaLog>& logOut);
	// T must implement:
	// bool T::readInstance(
	// 	const QUaNodeId& nodeId,
	// 	QString& typeName,
	// 	QMap<QString, QVariant>& attrs,
	// 	QList<QUaForwardReference>& forwardRefs,
	// 	QQueue<QUaLog>& logOut
	// );
	// T can optionally implement:
	// bool T::deserializeStart(QQueue<QUaLog>& logOut);
	// bool T::deserializeEnd(QQueue<QUaLog>& logOut);
	template<typename T>
	bool deserialize(T& deserializer, QQueue<QUaLog>& logOut);

	// Clone API

	QUaNode* cloneNode(
		QUaNode* parentNode = nullptr,
		const QUaQualifiedName& browseName = QUaQualifiedName(),
		const QUaNodeId& nodeId = QUaNodeId()
	);

signals:

	void displayNameChanged(const QUaLocalizedText& displayName);
	void descriptionChanged(const QUaLocalizedText& description);
	void writeMaskChanged(const quint32& writeMask);

	void childAdded(QUaNode* childNode);
	void referenceAdded(const QUaReferenceType &refType, QUaNode* nodeTarget, const bool& isForward);
	void referenceRemoved(const QUaReferenceType &refType, QUaNode* nodeTarget, const bool& isForward);

protected:
	// to be able to reuse methods in subclasses
	QUaServer* m_qUaServer;
	// to check which session is calling a service (read, write, method call, etc)
	const QUaSession* currentSession() const;
	// instatiate child with optional modelling rule
	QUaNode* instantiateOptionalChild(const QUaQualifiedName& browseName);
	// check if instance has an optional method with given browse name
	bool hasOptionalMethod(const QUaQualifiedName& methodName) const;
	// gets optional method from type and adds a reference from this instance to the method
	bool addOptionalMethod(const QUaQualifiedName& methodName);
	// removes the reference by the method above
	bool removeOptionalMethod(const QUaQualifiedName& methodName);
	// to check if a node is visible in the address space (reachible in hierarchical refs tree)
	bool inAddressSpace() const;

private:
	// INSTANCE NodeId
	UA_NodeId m_nodeId;
	// keep a cache since it does not change much
	QUaNodeId m_typeDefinitionNodeId;
	static QHash<QUaNodeId, QUaQualifiedName> m_hashTypeBrowseNames;
	// with new open62541, browseName is inmutable and reading it is kind of expensive
	QUaQualifiedName m_browseName;
	// browse hierarchical children cache
	// NOTE : use uint as key because is cheaper to cache in QObject::destroyed
	// lambda callback than the browseName. We need to cache because by the time the
	// QUaNode destructor is called, the browseName is already unavailable from open62541
	// TODO : consider removing after testing new open62541 tree implementation
	QHash<uint, QUaNode*> m_browseCache;

	// Static Helpers

	// NOTE : need to cleanup result after calling this method
	static UA_NodeId getParentNodeId(const UA_NodeId& childNodeId, UA_Server* server);
	// NOTE : need to cleanup result after calling this method
	static QList<UA_NodeId> getChildrenNodeIds(
		const UA_NodeId& parentNodeId, 
		UA_Server* server,
		const UA_UInt32 &nodeClassMask   = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE,
		const UA_NodeId &referenceTypeId = UA_NODEID_NULL
	);
	// NOTE : need to cleanup result after calling this method
	static QList<UA_NodeId> getMethodsNodeIds(const UA_NodeId& parentNodeId, UA_Server* server);

	// NOTE : needed to store historic events in a consistent way, ignoring manually added children
	//        decided to make key QString because use of QUaQualifiedName is more expensive for hash
	//        puposes. This might be an issue if there are two fields with same name but different
	//        namespace
	typedef QHash<QUaBrowsePath, QUaDataType> QUaEventFieldMetaData;
	typedef QHash<QUaBrowsePath, QUaDataType>::iterator QUaEventFieldMetaDataIter;
	static QUaEventFieldMetaData getTypeVars(
		const QUaNodeId& typeNodeId,
		UA_Server* server
	);

	static QUaNode* getNodeContext(const UA_NodeId& nodeId, UA_Server* server);
	static void*    getVoidContext(const UA_NodeId& nodeId, UA_Server* server);

	static QUaQualifiedName getBrowseName(const UA_NodeId& nodeId, UA_Server* server);

	static UA_NodeId getModellingRule    (const UA_NodeId& nodeId, UA_Server* server);
	static bool hasMandatoryModellingRule(const UA_NodeId& nodeId, UA_Server* server);
	static bool hasOptionalModellingRule (const UA_NodeId& nodeId, UA_Server* server);

	static int getPropsOffsetHelper(const QMetaObject& metaObject);
	// NOTE : need to cleanup result after calling this method
	static UA_NodeId typeDefinitionNodeId(const UA_NodeId &nodeId, UA_Server* server);
	static UA_NodeId superTypeDefinitionNodeId(const UA_NodeId &typeNodeId, UA_Server* server);

	static UA_StatusCode addOptionalVariableField(
		UA_Server*              server, 
		const UA_NodeId*        originNode,
		const UA_QualifiedName* fieldName,
		const UA_VariableNode*  optionalVariableFieldNode,
		UA_NodeId*              outOptionalVariable
	);
	static UA_StatusCode addOptionalObjectField(
		UA_Server*              server, 
		const UA_NodeId*        originNode,
		const UA_QualifiedName* fieldName,
		const UA_ObjectNode*    optionalObjectFieldNode,
		UA_NodeId*              outOptionalObject);

	static UA_NodeId getOptionalChildNodeId(
		UA_Server* server, 
		const UA_NodeId& typeNodeId, 
		const UA_QualifiedName& browseName
	);
	static QUaNode * instantiateOptionalChild(
		UA_Server* server,
		QUaNode * parent,
		const UA_NodeId& optionalFieldNodeId,
		const UA_QualifiedName &childName
	);

	QSet<UA_NodeId> getRefsInternal(const QUaReferenceType& ref, const bool& isForward = true) const;
	// NOTE : need internal because user might reimplement public
	QUaWriteMask   userWriteMaskInternal(const QString& strUserName);
	QUaAccessLevel userAccessLevelInternal(const QString& strUserName);
	bool           userExecutableInternal(const QString& strUserName);

	// Serialization API
	const QMap<QString, QVariant>    serializeAttrs() const;
	const QList<QUaForwardReference> serializeRefs() const;

	template<typename T>
	bool serializeInternal(T& serializer, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<QUaHasMethodSerializeStart<T>::value, bool>::type
	serializeStart(T& serializer, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<!QUaHasMethodSerializeStart<T>::value, bool>::type
	serializeStart(T& serializer, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<QUaHasMethodSerializeEnd<T>::value, bool>::type
	serializeEnd(T& serializer, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<!QUaHasMethodSerializeEnd<T>::value, bool>::type
	serializeEnd(T& serializer, QQueue<QUaLog>& logOut);

	template<typename T>
	bool deserializeInternal(T& deserializer, 
		                     const QMap<QString, QVariant>& attrs, 
		                     const QList<QUaForwardReference>& forwardRefs, 
		                     QMap<QUaNode*, QList<QUaForwardReference>>& nonHierRefs, 
		                     QQueue<QUaLog>& logOut, 
		                     const bool &isObjsFolder = false);
	void deserializeAttrs(const QMap<QString, QVariant>& attrs, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<QUaHasMethodDeserializeStart<T>::value, bool>::type
	deserializeStart(T& serializer, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<!QUaHasMethodDeserializeStart<T>::value, bool>::type
	deserializeStart(T& serializer, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<QUaHasMethodDeserializeEnd<T>::value, bool>::type
	deserializeEnd(T& deserializer, QQueue<QUaLog>& logOut);

	template<typename T>
	typename std::enable_if<!QUaHasMethodDeserializeEnd<T>::value, bool>::type
	deserializeEnd(T& deserializer, QQueue<QUaLog>& logOut);

	std::function<QUaWriteMask(const QString&)> m_userWriteMaskCallback;
	std::function<QUaAccessLevel(const QString&)> m_userAccessLevelCallback;
	std::function<bool(const QString&)> m_userExecutableCallback;
};

template<typename T>
inline QList<T*> QUaNode::browseChildren() const
{
	QList<T*> retList;
	// call QUaNode specialization
	auto originalList = this->browseChildren();
	// filter out the ones that downcast to T*
	for (int i = 0; i < originalList.count(); i++)
	{
		T* possible = dynamic_cast<T*>(originalList.at(i));
		if (possible)
		{
			retList.append(possible);
		}
	}
	return retList;
}

template<typename T>
inline T* QUaNode::browseChild(
	const QUaQualifiedName& browseName,
	const bool& instantiateOptional)
{
	return qobject_cast<T*>(this->browseChild(browseName, instantiateOptional));
}

template<typename T>
inline T* QUaNode::browsePath(const QUaBrowsePath& browsePath) const
{
	return qobject_cast<T*>(this->browsePath(browsePath));
}

template<typename T>
inline QList<T*> QUaNode::findReferences(const QUaReferenceType&ref, const bool &isForward/* = true*/) const
{
	QList<T*> retList;
	QList<QUaNode*> nodeList = findReferences(ref, isForward);
	for (int i = 0; i < nodeList.count(); i++)
	{
		T* ref = qobject_cast<T*>(nodeList.at(i));
		if (ref)
		{
			retList.append(ref);
		}
	}
	return retList;
}

template<typename M>
inline void QUaNode::setUserWriteMaskCallback(const M & callback)
{
	m_userWriteMaskCallback = [callback](const QString &strUserName) {
		return callback(strUserName);
	};
}

template<typename M>
inline void QUaNode::setUserAccessLevelCallback(const M & callback)
{
	m_userAccessLevelCallback = [callback](const QString &strUserName) {
		return callback(strUserName);
	};
}

template<typename M>
inline void QUaNode::setUserExecutableCallback(const M & callback)
{
	m_userExecutableCallback = [callback](const QString &strUserName) {
		return callback(strUserName);
	};
}

#endif // QUASERVERNODE_H

