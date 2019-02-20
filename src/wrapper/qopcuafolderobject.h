#ifndef QOPCUAFOLDEROBJECT_H
#define QOPCUAFOLDEROBJECT_H

#include <QOpcUaBaseObject>

class QOpcUaBaseDataVariable;

// Part 5 - 6.6 : FolderType

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
class QOpcUaNodeFactory<QOpcUaFolderObject>
{
public:
	static UA_NodeId GetTypeNodeId()
	{
		return UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);
	}

	static void SetTypeNodeId(const UA_NodeId & typeNodeId)
	{
		Q_UNUSED(typeNodeId);
	}
};

#endif // QOPCUAFOLDEROBJECT_H