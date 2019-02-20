#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaAbstractVariable>

// Part 5 - 7.2 : BaseVariableType

class QOpcUaBaseVariable : public QOpcUaAbstractVariable
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariable(QOpcUaServerNode *parent);

	
};

// [TRAITS] : specialization
template <>
class QOpcUaNodeFactory<QOpcUaBaseVariable>
{
public:
	static UA_NodeId GetTypeNodeId()
	{
		return UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE);
	}

	static void SetTypeNodeId(const UA_NodeId & typeNodeId)
	{
		Q_UNUSED(typeNodeId);
	}
};


#endif // QOPCUABASEVARIABLE_H