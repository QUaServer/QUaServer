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
	Q_INVOKABLE explicit QOpcUaBaseDataVariable(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject);

	// Instance Creation API
    // NOTE : implemented in qopcuaserver.h to avoid compiler errors
	template<typename T>
	T* addChild();

};

#endif // QOPCUABASEDATAVARIABLE_H

