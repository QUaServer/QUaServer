#ifndef QUACONDITIONVARIABLE_H
#define QUACONDITIONVARIABLE_H

#include <QUaBaseDataVariable>

// Part X - X.X : ConditionVariableType
/*
The ConditionVariableType is a subtype of the BaseDataVariableType. 

*/

class QUaConditionVariable : public QUaBaseDataVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaConditionVariable(QUaServer *server);


};

#endif // QUACONDITIONVARIABLE_H

