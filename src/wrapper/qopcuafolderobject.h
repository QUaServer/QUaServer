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

class QOpcUaFolderObject : public QOpcUaBaseObject, public QOpcUaServerNodeFactory<QOpcUaFolderObject>
{
    Q_OBJECT

friend class QOpcUaServer;

public:
	Q_INVOKABLE explicit QOpcUaFolderObject(QOpcUaServerNode *parent);

	

private:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaFolderObject(QOpcUaServer *server);
};

#endif // QOPCUAFOLDEROBJECT_H