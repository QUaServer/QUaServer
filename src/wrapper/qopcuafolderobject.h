#ifndef QOPCUAFOLDEROBJECT_H
#define QOPCUAFOLDEROBJECT_H

#include <QOpcUaBaseObject>

class QOpcUaBaseDataVariable;

// Part 5 - 6.6 : FolderType
/*
Instances of this ObjectType are used to organise the AddressSpace into 
a hierarchy of Nodes. They represent the root Node of a subtree, and have 
no other semantics associated with them. However, the DisplayName of an 
instance of the FolderType, such as “ObjectTypes”, should imply the
semantics associated with the use of it.
*/

class QOpcUaFolderObject : public QOpcUaBaseObject
{
    Q_OBJECT

friend class QOpcUaServer;

public:
    explicit QOpcUaFolderObject(QOpcUaServerNode *parent);

	

private:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaFolderObject(QOpcUaServer *server);
};

// [TRAITS] : specialization
template <>
struct QOpcUaNodeFactory<QOpcUaFolderObject>
{
	static UA_NodeId GetTypeNodeId()
	{
		return UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
	}

	static void SetTypeNodeId(const UA_NodeId & typeNodeId)
	{
		Q_UNUSED(typeNodeId);
	}

	static QString GetDisplayName()
	{
		return QString();
	}

	static QString GetDescription()
	{
		return QString();
	}

	static quint32 GetWriteMask()
	{
		return 0;
	}
};

#endif // QOPCUAFOLDEROBJECT_H