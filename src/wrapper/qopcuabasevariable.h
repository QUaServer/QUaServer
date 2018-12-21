#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaNodeFactory>

class QOpcUaBaseVariable : public QOpcUaServerNode, public QOpcUaNodeFactory<QOpcUaBaseVariable>
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariable(QOpcUaServerNode *parent);


};

#endif // QOPCUABASEVARIABLE_H