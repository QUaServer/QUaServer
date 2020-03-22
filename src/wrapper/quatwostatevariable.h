#ifndef QUATWOSTATEVARIABLE_H
#define QUATWOSTATEVARIABLE_H

#include <QUaStateVariable>

// Part X - X.X : StateVariableType
/*
The TwoStateVariableType is a subtype of the StateVariableType. 

*/

class QUaTwoStateVariable : public QUaStateVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaTwoStateVariable(QUaServer *server);


};

#endif // QUATWOSTATEVARIABLE_H

