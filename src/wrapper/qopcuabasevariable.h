#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaAbstractVariable>

// Part 5 - 7.2 : BaseVariableType
/*
The BaseVariableType is the abstract base type for all other VariableTypes. 
However, only the PropertyType and the BaseDataVariableType directly inherit from this type.
*/

class QOpcUaBaseVariable : public QOpcUaAbstractVariable
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariable(QOpcUaServerNode *parent);

	
};

// [TRAITS] : specialization
template <>
struct QOpcUaNodeFactory<QOpcUaBaseVariable>
{
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