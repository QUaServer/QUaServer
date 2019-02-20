#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaAbstractVariable>

// Part 5 - 7.2 : BaseVariableType

class QOpcUaBaseVariable : public QOpcUaAbstractVariable, public QOpcUaNodeFactory<QOpcUaBaseVariable>
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariable(QOpcUaServerNode *parent);

	
};

#endif // QOPCUABASEVARIABLE_H