#ifndef QUABASEDATAVARIABLE_H
#define QUABASEDATAVARIABLE_H

#include <QUaBaseVariable>

// Part 5 - 7.4 : BaseDataVariableType
/*
The BaseDataVariableType is a subtype of the BaseVariableType. 
It is used as the type definition whenever there is a DataVariable having no more concrete type definition available. 
This VariableType is the base VariableType for VariableTypes of DataVariables, and all other VariableTypes of DataVariables 
shall either directly or indirectly inherit from it.
*/

class QUaBaseDataVariable : public QUaBaseVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaBaseDataVariable(
		QUaServer* server
	);

	// Instance Creation API
    // NOTE : implemented in qopcuaserver.h to avoid compiler errors
	template<typename T>
	T* addChild(const QString &strNodeId = "");

};

#endif // QUABASEDATAVARIABLE_H

