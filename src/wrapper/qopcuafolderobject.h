#ifndef QOPCUAFOLDEROBJECT_H
#define QOPCUAFOLDEROBJECT_H

#include <QOpcUaBaseObject>

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
	Q_INVOKABLE explicit QOpcUaFolderObject(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	

};

#endif // QOPCUAFOLDEROBJECT_H