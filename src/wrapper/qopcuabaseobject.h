#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QOpcUaNodeFactory>

/*
typedef struct {                          // UA_ObjectTypeAttributes_default
	// Node Attributes
	UA_UInt32        specifiedAttributes; // 0,
	UA_LocalizedText displayName;         // {{0, NULL}, {0, NULL}},
	UA_LocalizedText description;         // {{0, NULL}, {0, NULL}},
	UA_UInt32        writeMask;           // 0,
	UA_UInt32        userWriteMask;       // 0,
	// Object Type Attributes
	UA_Boolean       isAbstract;          // false
} UA_ObjectTypeAttributes;

typedef struct {                          // UA_ObjectAttributes_default
	// Node Attributes
	UA_UInt32        specifiedAttributes; // 0,
	UA_LocalizedText displayName;         // {{0, NULL}, {0, NULL}},
	UA_LocalizedText description;         // {{0, NULL}, {0, NULL}},
	UA_UInt32        writeMask;           // 0,
	UA_UInt32        userWriteMask;       // 0,
	// Object Attributes
	UA_Byte          eventNotifier;       // 0
} UA_ObjectAttributes;
*/

class QOpcUaProperty;
class QOpcUaBaseDataVariable;
class QOpcUaFolderObject;

// Part 5 - 6.2 : BaseObjectType
/*
The BaseObjectType is used as type definition whenever there is an Object 
having no more concrete type definitions available. 
Servers should avoid using this ObjectType and use a more specific type, if possible. 
This ObjectType is the base ObjectType and all other ObjectTypes shall either 
directly or indirectly inherit from it.
*/

class QOpcUaBaseObject : public QOpcUaServerNode
{
    Q_OBJECT

	// Object Attributes

	// TODO
	//Q_PROPERTY(UA_Byte eventNotifier READ get_eventNotifier)

friend class QOpcUaServer;

public:
	explicit QOpcUaBaseObject(QOpcUaServerNode *parent);

	QOpcUaProperty         * addProperty(const QString &strBrowseName = "");

	QOpcUaBaseDataVariable * addBaseDataVariable(const QString &strBrowseName = "");

	QOpcUaBaseObject       * addBaseObject(const QString &strBrowseName = "");

	QOpcUaFolderObject     * addFolderObject(const QString &strBrowseName = "");

protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaBaseObject(QOpcUaServer *server);
};

// [TRAITS] : specialization
template <>
struct QOpcUaNodeFactory<QOpcUaBaseObject>
{
	static UA_NodeId GetTypeNodeId()
	{
		return UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
	}

	static void SetTypeNodeId(const UA_NodeId & typeNodeId)
	{
		Q_UNUSED(typeNodeId);
	}
};

#endif // QOPCUABASEOBJECT_H

