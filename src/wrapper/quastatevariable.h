#ifndef QUASTATEVARIABLE_H
#define QUASTATEVARIABLE_H

#include <QUaBaseDataVariable>

// Part X - X.X : StateVariableType
/*
The StateVariableType is a subtype of the BaseDataVariableType. 

*/

class QUaStateVariable : public QUaBaseDataVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaStateVariable(
		QUaServer* server,
		const MC& mandatoryChildren = &QUaStateVariable::mandatoryChildrenBrowseNames
	);


};

#endif // QUASTATEVARIABLE_H

