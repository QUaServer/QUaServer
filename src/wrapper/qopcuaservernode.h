#ifndef QOPCUASERVERNODE_H
#define QOPCUASERVERNODE_H

#include <functional>
#include <typeinfo>

#include <QObject>
#include <QVariant>
#include <QMetaProperty>

#include "open62541.h"

class QOpcUaServer;
class QOpcUaProperty;
class QOpcUaBaseDataVariable;
class QOpcUaBaseObject;
class QOpcUaFolderObject;

#include <QOpcUaTypesConverter>

// traits used to static assert that a method is not used
// https://stackoverflow.com/questions/24609872/delete-virtual-function-from-a-derived-class
template <typename T>
struct QOpcUaFail : std::false_type
{
};

// to have UA_NodeId as a hash key
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

/*
typedef struct {
	// Node Attributes
	UA_UInt32        specifiedAttributes;
	UA_LocalizedText displayName;
	UA_LocalizedText description;
	UA_UInt32        writeMask;
	UA_UInt32        userWriteMask;
} UA_NodeAttributes;
*/

class QOpcUaServerNode : public QObject
{
    Q_OBJECT

	// Node Attributes

	// N/A
	//Q_PROPERTY(quint32 specifiedAttributes READ get_specifiedAttributes)

	Q_PROPERTY(QString displayName READ get_displayName WRITE set_displayName NOTIFY displayNameChanged)
	Q_PROPERTY(QString description READ get_description WRITE set_description NOTIFY descriptionChanged)
	Q_PROPERTY(quint32 writeMask   READ get_writeMask   WRITE set_writeMask   NOTIFY writeMaskChanged  )

	// Cannot be read, since the local "admin" user always has full rights.
	// Cannot be written from the server, as they are specific to the different users and set by the access control callback.
	//Q_PROPERTY(quint32 userWriteMask READ get_userWriteMask)

	// Node Specifics

	// Cannot be changed once a node has been created.
	Q_PROPERTY(QString nodeId     READ get_nodeId   )
	Q_PROPERTY(QString nodeClass  READ get_nodeClass)

	// Other

    Q_PROPERTY(QString browseName READ get_browseName WRITE set_browseName NOTIFY browseNameChanged)

public:

	// OPC UA methods API

	QString get_displayName() const;
	void    set_displayName(const QString &displayName);
	QString get_description() const;
	void    set_description(const QString &description);
	quint32 get_writeMask  () const;
	void    set_writeMask  (const quint32 &writeMask);

	QString get_nodeId     () const;
	QString get_nodeClass  () const;
	
	QString get_browseName() const;
    void    set_browseName(const QString &browseName);

	// Instance Creation API

	virtual QOpcUaProperty         * addProperty        ();
	virtual QOpcUaBaseDataVariable * addBaseDataVariable();
	virtual QOpcUaBaseObject       * addBaseObject      ();
	virtual QOpcUaFolderObject     * addFolderObject    ();


	// to be able to reuse methods in subclasses
	QOpcUaServer * m_qopcuaserver;

	// protected?

	// INSTANCE NodeId
	UA_NodeId m_nodeId;

	// private
	// NOTE : this is how we would declare a <template class> friend
	//template<typename T> friend class QOpcUaServerNodeFactory;
	UA_Server * getUAServer();
	void        bindWithUaNode(QOpcUaServer *server, const UA_NodeId &nodeId);

	static UA_NodeId getParentNodeId(const UA_NodeId &childNodeId, QOpcUaServer *server);
	static UA_NodeId getParentNodeId(const UA_NodeId &childNodeId, UA_Server    *server);

	static QList<UA_NodeId> getChildrenNodeIds(const UA_NodeId &parentNodeId, QOpcUaServer *server);
	static QList<UA_NodeId> getChildrenNodeIds(const UA_NodeId &parentNodeId, UA_Server    *server);

	static QOpcUaServerNode * getNodeContext(const UA_NodeId &nodeId, QOpcUaServer *server);
	static QOpcUaServerNode * getNodeContext(const UA_NodeId &nodeId, UA_Server    *server);

	static int getPropsOffsetHelper(const QMetaObject &metaObject);

signals:

	void displayNameChanged(const QString &displayName);
	void descriptionChanged(const QString &description);
	void writeMaskChanged  (const quint32 &writeMask  );
	void browseNameChanged (const QString &browseName );
	

private:


};

template<typename T>
struct QOpcUaServerNodeFactory
{
	// For custom obj types with props
	inline QOpcUaServerNodeFactory(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL)
	{
		// check
		if (!server || UA_NodeId_isNull(&nodeId))
		{
			return;
		}
		// get instance of type being instantiated
		auto ptrThis = static_cast<T*>(this);
		// bind itself
		ptrThis->bindWithUaNode(server, nodeId);

		// get all UA children in advance, because if none, then better early exit
		auto chidrenNodeIds = QOpcUaServerNode::getChildrenNodeIds(nodeId, server);
		if (chidrenNodeIds.count() <= 0)
		{
			return;
		}
		// create map with nodeId's context which already must be valid QOpcUaServerNode instances
		QHash<UA_NodeId, QOpcUaServerNode*> mapChildren;
		for (int i = 0; i < chidrenNodeIds.count(); i++)
		{
			auto childNodeId = chidrenNodeIds[i];
			Q_ASSERT(!mapChildren.contains(childNodeId));
			mapChildren[childNodeId] = QOpcUaServerNode::getNodeContext(childNodeId, server);
		}
		
		
		// [DEBUG]
		qDebug() << "QOpcUaServerNodeFactory<" << T::staticMetaObject.className() << ">";

		// list meta props
		auto metaObj   = T::staticMetaObject;
		int propCount  = metaObj.propertyCount();
		int propOffset = QOpcUaServerNode::getPropsOffsetHelper(T::staticMetaObject);


		for (int i = propOffset; i < propCount; i++)
		{
			// check if available in meta-system
			QMetaProperty metaproperty = T::staticMetaObject.property(i);
			if (!QMetaType::metaObjectForType(metaproperty.userType()))
			{
				continue;
			}
			// check if OPC UA relevant type
			const QMetaObject propMetaObject = *QMetaType::metaObjectForType(metaproperty.userType());
			if (!propMetaObject.inherits(&QOpcUaServerNode::staticMetaObject))
			{
				continue;
			}
			// check if prop inherits from parent
			Q_ASSERT_X(!propMetaObject.inherits(&T::staticMetaObject), "QOpcUaServerNodeFactory", "Qt MetaProperty type cannot inherit from Class.");
			if (propMetaObject.inherits(&T::staticMetaObject))
			{
				continue;
			}

			// [DEBUG]
			qDebug() << QString(metaproperty.name());



			// get pointer to node-derived instance, must be already instantiated
			//metaproperty.write(ptrThis, );

			// TODO : 

			



		}

		
	}
};

#endif // QOPCUASERVERNODE_H

