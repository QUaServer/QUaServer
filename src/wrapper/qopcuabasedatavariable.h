#ifndef QOPCUABASEDATAVARIABLE_H
#define QOPCUABASEDATAVARIABLE_H

#include <QOpcUaBaseVariable>

// Part 5 - 7.4 : BaseDataVariableType
/*
The BaseDataVariableType is a subtype of the BaseVariableType. 
It is used as the type definition whenever there is a DataVariable having no more concrete type definition available. 
This VariableType is the base VariableType for VariableTypes of DataVariables, and all other VariableTypes of DataVariables 
shall either directly or indirectly inherit from it.
*/

class QOpcUaBaseDataVariable : public QOpcUaBaseVariable
{
    Q_OBJECT
public:
    explicit QOpcUaBaseDataVariable(QOpcUaServerNode *parent);

	
};

// [TRAITS] : specialization
template <>
struct QOpcUaNodeFactory<QOpcUaBaseDataVariable>
{
	static UA_NodeId GetTypeNodeId()
	{
		return UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	}

	static void SetTypeNodeId(const UA_NodeId & typeNodeId)
	{
		Q_UNUSED(typeNodeId);
	}
};


#endif // QOPCUABASEDATAVARIABLE_H