#ifndef QOPCUAPROPERTY_H
#define QOPCUAPROPERTY_H

#include <QOpcUaBaseVariable>

// Part 5 - 7.3 : PropertyType
/*
The PropertyType is a subtype of the BaseVariableType. 
It is used as the type definition for all Properties. 
Properties are defined by their BrowseName and therefore they do not need 
a specialised type definition. It is not allowed to subtype this VariableType.
*/

class QOpcUaProperty : public QOpcUaBaseVariable
{
    Q_OBJECT
public:
    explicit QOpcUaProperty(QOpcUaServerNode *parent);

	
};

// [TRAITS] : specialization
template <>
struct QOpcUaNodeFactory<QOpcUaProperty>
{
	static UA_NodeId GetTypeNodeId()
	{
		return UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE);
	}

	static void SetTypeNodeId(const UA_NodeId & typeNodeId)
	{
		Q_UNUSED(typeNodeId);
	}
};


#endif // QOPCUAPROPERTY_H