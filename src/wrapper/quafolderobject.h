#ifndef QUAFOLDEROBJECT_H
#define QUAFOLDEROBJECT_H

#include <QUaBaseObject>

// Part 5 - 6.6 : FolderType
/*
Instances of this ObjectType are used to organise the AddressSpace into 
a hierarchy of Nodes. They represent the root Node of a subtree, and have 
no other semantics associated with them. However, the DisplayName of an 
instance of the FolderType, such as “ObjectTypes”, should imply the
semantics associated with the use of it.
*/

class QUaFolderObject : public QUaBaseObject
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaFolderObject(QUaServer *server);

	

};

#endif // QUAFOLDEROBJECT_H