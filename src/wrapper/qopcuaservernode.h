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
	return e1.namespaceIndex     == e2.namespaceIndex
		&& e1.identifierType     == e2.identifierType
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

	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(quint32 writeMask   READ writeMask   WRITE setWriteMask   NOTIFY writeMaskChanged  )

	// Cannot be read, since the local "admin" user always has full rights.
	// Cannot be written from the server, as they are specific to the different users and set by the access control callback.
	//Q_PROPERTY(quint32 userWriteMask READ get_userWriteMask)

	// Node Specifics

	// Cannot be changed once a node has been created.
	Q_PROPERTY(QString nodeId     READ nodeId   )
	Q_PROPERTY(QString nodeClass  READ nodeClass)

	// Other

    Q_PROPERTY(QString browseName READ browseName WRITE setBrowseName NOTIFY browseNameChanged)

public:
	explicit QOpcUaServerNode(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject);
	
	// Virtual destructor is necessary to call derived class destructor when delete is called on pointer to base class
	// https://stackoverflow.com/questions/294927/does-delete-work-with-pointers-to-base-class
	// https://repl.it/repls/EachSpicyInternalcommand
	inline virtual ~QOpcUaServerNode() { };

	// OPC UA methods API

	QString displayName   () const;
	void    setDisplayName(const QString &displayName);
	QString description   () const;
	void    setDescription(const QString &description);
	quint32 writeMask     () const;
	void    setWriteMask  (const quint32 &writeMask);

	QString nodeId        () const;
	QString nodeClass     () const;
	
	QString browseName    () const;
    void    setBrowseName (const QString &browseName);

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
	UA_Server * getUAServer();

	static UA_NodeId getParentNodeId(const UA_NodeId &childNodeId, QOpcUaServer *server);
	static UA_NodeId getParentNodeId(const UA_NodeId &childNodeId, UA_Server    *server);

	static QList<UA_NodeId> getChildrenNodeIds(const UA_NodeId &parentNodeId, QOpcUaServer *server);
	static QList<UA_NodeId> getChildrenNodeIds(const UA_NodeId &parentNodeId, UA_Server    *server);

	static QOpcUaServerNode * getNodeContext(const UA_NodeId &nodeId, QOpcUaServer *server);
	static QOpcUaServerNode * getNodeContext(const UA_NodeId &nodeId, UA_Server    *server);
	static void             * getVoidContext(const UA_NodeId &nodeId, UA_Server    *server);

	static QString getBrowseName (const UA_NodeId &nodeId, QOpcUaServer *server);
	static QString getBrowseName (const UA_NodeId &nodeId, UA_Server    *server);

	static int getPropsOffsetHelper(const QMetaObject &metaObject);

signals:

	void displayNameChanged(const QString &displayName);
	void descriptionChanged(const QString &description);
	void writeMaskChanged  (const quint32 &writeMask  );
	void browseNameChanged (const QString &browseName );
	

private:


};

#endif // QOPCUASERVERNODE_H

