#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QOpcUaAbstractObject>

class QOpcUaBaseDataVariable;
class QOpcUaFolderObject;

// Part 5 - 6.2 : BaseObjectType

class QOpcUaBaseObject : public QOpcUaAbstractObject
{
    Q_OBJECT

friend class QOpcUaServer;

public:
	explicit QOpcUaBaseObject(QOpcUaServerNode *parent);

	QOpcUaBaseObject       * addBaseObject();

	QOpcUaBaseDataVariable * addBaseDataVariable();

	QOpcUaFolderObject     * addFolderObject();

protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaBaseObject(QOpcUaServer *server);
};

// [TRAITS] : specialization
template <>
class QOpcUaNodeFactory<QOpcUaBaseObject>
{
public:
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

