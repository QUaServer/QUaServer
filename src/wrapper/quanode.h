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

struct QUaReference
{
	QString strForwardName;
	QString strInverseName;
};

// to have QUaReference as a hash key
inline bool operator==(const QUaReference &e1, const QUaReference &e2)
{
	return e1.strForwardName.compare(e2.strForwardName, Qt::CaseSensitive) == 0
		&& e1.strInverseName.compare(e2.strInverseName, Qt::CaseSensitive) == 0;
}

inline uint qHash(const QUaReference &key, uint seed)
{
	return qHash(key.strForwardName, seed) ^ qHash(key.strInverseName, seed);
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

union QUaWriteMask
{
	struct bit_map {
		bool bAccessLevel             : 1; // UA_WRITEMASK_ACCESSLEVEL            
		bool bArrrayDimensions        : 1; // UA_WRITEMASK_ARRRAYDIMENSIONS       
		bool bBrowseName              : 1; // UA_WRITEMASK_BROWSENAME             
		bool bContainsNoLoops         : 1; // UA_WRITEMASK_CONTAINSNOLOOPS        
		bool bDataType                : 1; // UA_WRITEMASK_DATATYPE               
		bool bDescription             : 1; // UA_WRITEMASK_DESCRIPTION            
		bool bDisplayName             : 1; // UA_WRITEMASK_DISPLAYNAME            
		bool bEventNotifier           : 1; // UA_WRITEMASK_EVENTNOTIFIER          
		bool bExecutable              : 1; // UA_WRITEMASK_EXECUTABLE             
		bool bHistorizing             : 1; // UA_WRITEMASK_HISTORIZING            
		bool bInverseName             : 1; // UA_WRITEMASK_INVERSENAME            
		bool bIsAbstract              : 1; // UA_WRITEMASK_ISABSTRACT             
		bool bMinimumSamplingInterval : 1; // UA_WRITEMASK_MINIMUMSAMPLINGINTERVAL
		bool bNodeClass               : 1; // UA_WRITEMASK_NODECLASS              
		bool bNodeId                  : 1; // UA_WRITEMASK_NODEID                 
		bool bSymmetric               : 1; // UA_WRITEMASK_SYMMETRIC              
		bool bUserAccessLevel         : 1; // UA_WRITEMASK_USERACCESSLEVEL        
		bool bUserExecutable          : 1; // UA_WRITEMASK_USEREXECUTABLE         
		bool bUserWriteMask           : 1; // UA_WRITEMASK_USERWRITEMASK          
		bool bValueRank               : 1; // UA_WRITEMASK_VALUERANK              
		bool bWriteMask               : 1; // UA_WRITEMASK_WRITEMASK              
		bool bValueForVariableType    : 1; // UA_WRITEMASK_VALUEFORVARIABLETYPE  
	} bits;
	quint32 intValue;
	// constructors
	QUaWriteMask()
	{
		// all attributes writable by default (getUserRightsMask_default returns 0xFFFFFFFF)
		bits.bAccessLevel             = true;
		bits.bArrrayDimensions        = true;
		bits.bBrowseName              = true;
		bits.bContainsNoLoops         = true;
		bits.bDataType                = true;
		bits.bDescription             = true;
		bits.bDisplayName             = true;
		bits.bEventNotifier           = true;
		bits.bExecutable              = true;
		bits.bHistorizing             = true;
		bits.bInverseName             = true;
		bits.bIsAbstract              = true;
		bits.bMinimumSamplingInterval = true;
		bits.bNodeClass               = true;
		bits.bNodeId                  = true;
		bits.bSymmetric               = true;
		bits.bUserAccessLevel         = true;
		bits.bUserExecutable          = true;
		bits.bUserWriteMask           = true;
		bits.bValueRank               = true;
		bits.bWriteMask               = true;
		bits.bValueForVariableType    = true;
	};
	QUaWriteMask(const quint32 &value)
	{
		intValue = value;
	};
};

class QUaNode : public QObject
{
    Q_OBJECT

	// Node Attributes

	// N/A : not an OPC UA node attribute, is it a library helper?
	//Q_PROPERTY(quint32 specifiedAttributes READ get_specifiedAttributes)

	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	// Exposes the possibilities of a client to write the Attributes of the Node.
	Q_PROPERTY(quint32 writeMask   READ writeMask   WRITE setWriteMask   NOTIFY writeMaskChanged  )

	// Cannot be read, since the local API user always has full rights.
	// Cannot be written from the server, as they are specific to the different users and set by the access control callback.
	// It is defined by overwriting the server's config->accessControl.getUserRightsMask (see getUserRightsMask_default)
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

	bool operator ==(const QUaNode &other) const;

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

	virtual QUaProperty         * addProperty        (const QString &strNodeId = "");
	virtual QUaBaseDataVariable * addBaseDataVariable(const QString &strNodeId = "");
	virtual QUaBaseObject       * addBaseObject      (const QString &strNodeId = "");
	virtual QUaFolderObject     * addFolderObject    (const QString &strNodeId = "");

	// Reference API

	void addReference(const QUaReference &ref, const QUaNode * nodeTarget, const bool &isForward = true);

	void removeReference(const QUaReference &ref, const QUaNode * nodeTarget, const bool &isForward = true);

	template<typename T>
	QList<T*>       getReferences(const QUaReference &ref, const bool &isForward = true);
	// specialization
	QList<QUaNode*> getReferences(const QUaReference &ref, const bool &isForward = true);

	// Access Control API

	// reimplement for custom access control for a given custom UA type (derived C++ class)
	virtual QUaWriteMask userWriteMask(const QString &strUserName);

	// provide specific implementation for individual nodes
	// signature is <QUaWriteMask(const QString &, QUaNode *)>
	template<typename M>
	void setUserWriteMaskCallback(const M &callback);

	// Helpers



	// Static Helpers

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

	QSet<UA_NodeId> getRefsInternal(const QUaReference &ref, const bool &isForward = true);

	std::function<QUaWriteMask(const QString &)> m_userWriteMaskCallback;

};

template<typename T>
inline QList<T*> QUaNode::getReferences(const QUaReference &ref, const bool &isForward/* = true*/)
{
	QList<T*> retList;
	QList<QUaNode*> nodeList = getReferences(ref, isForward);
	for (int i = 0; i < nodeList.count(); i++)
	{
		T* ref = dynamic_cast<T*>(nodeList.at(i));
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
	m_userWriteMaskCallback = [callback, this](const QString &strUserName) {
		return callback(strUserName, this);
	};
}

#endif // QUASERVERNODE_H

