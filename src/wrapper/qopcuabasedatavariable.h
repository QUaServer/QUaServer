#ifndef QOPCUABASEDATAVARIABLE_H
#define QOPCUABASEDATAVARIABLE_H

#include <QOpcUaBaseVariable>

class QOpcUaBaseDataVariable : public QOpcUaBaseVariable, public QOpcUaNodeFactory<QOpcUaBaseDataVariable>
{
    Q_OBJECT
public:
    explicit QOpcUaBaseDataVariable(QOpcUaServerNode *parent);


};

#endif // QOPCUABASEDATAVARIABLE_H