#ifndef QUASERVERNODE_H
#define QUASERVERNODE_H

#include <functional>
#include <typeinfo>

#include <QObject>
#include <QVariant>
#include <QMetaProperty>

#include "open62541.h"

class QUaServer;
class QUaProperty;
class QUaBaseDataVariable;
class QUaBaseObject;
class QUaFolderObject;

#include <QUaTypesConverter>

// traits used to static assert that a method cannot be used
// https://stackoverflow.com/questions/24609872/delete-virtual-function-from-a-derived-class
template <typename T>
struct QUaFail : std::false_type
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

class QUaNode : public QObject
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
	explicit QUaNode(QUaServer *server);
	
	// Virtual destructor is necessary to call derived class destructor when delete is called on pointer to base class
	// this can be useful when the library must delete a node because it was requested through the network
	// in which case we only have available a pointer to the base class, mainly because we support the user deriving 
	// from the library classes and we want to also call their custom destructors.
	// https://stackoverflow.com/questions/294927/does-delete-work-with-pointers-to-base-class
	// https://repl.it/repls/EachSpicyInternalcommand
	virtual ~QUaNode();

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

	virtual QUaProperty         * addProperty        ();
	virtual QUaBaseDataVariable * addBaseDataVariable();
	virtual QUaBaseObject       * addBaseObject      ();
	virtual QUaFolderObject     * addFolderObject    ();

	static UA_NodeId getParentNodeId(const UA_NodeId &childNodeId, QUaServer *server);
	static UA_NodeId getParentNodeId(const UA_NodeId &childNodeId, UA_Server *server);

	static QList<UA_NodeId> getChildrenNodeIds(const UA_NodeId &parentNodeId, QUaServer *server);
	static QList<UA_NodeId> getChildrenNodeIds(const UA_NodeId &parentNodeId, UA_Server *server);

	static QUaNode * getNodeContext(const UA_NodeId &nodeId, QUaServer *server);
	static QUaNode * getNodeContext(const UA_NodeId &nodeId, UA_Server *server);
	static void    * getVoidContext(const UA_NodeId &nodeId, UA_Server *server);

	static QString getBrowseName (const UA_NodeId &nodeId, QUaServer *server);
	static QString getBrowseName (const UA_NodeId &nodeId, UA_Server *server);

	static int getPropsOffsetHelper(const QMetaObject &metaObject);

	// protected, private?

	// to be able to reuse methods in subclasses
	QUaServer * m_qUaServer;
	// INSTANCE NodeId
	UA_NodeId m_nodeId;

signals:

	void displayNameChanged(const QString &displayName);
	void descriptionChanged(const QString &description);
	void writeMaskChanged  (const quint32 &writeMask  );
	void browseNameChanged (const QString &browseName );
	
	void childAdded(QUaNode * childNode);

private:


};

#endif // QUASERVERNODE_H

