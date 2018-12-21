#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaAbstractVariable>

class QOpcUaBaseVariable : public QOpcUaAbstractVariable, public QOpcUaNodeFactory<QOpcUaBaseVariable>
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariable(QOpcUaServerNode *parent);

	
};

#endif // QOPCUABASEVARIABLE_H