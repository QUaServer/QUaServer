#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaServerNode>

class QOpcUaBaseVariable : public QOpcUaServerNode
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariable(QOpcUaServerNode *parent);

signals:

public slots:
};

#endif // QOPCUABASEVARIABLE_H