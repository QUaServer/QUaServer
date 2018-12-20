#ifndef QOPCUABASEDATAVARIABLE_H
#define QOPCUABASEDATAVARIABLE_H

#include <QOpcUaBaseVariable>

class QOpcUaBaseDataVariable : public QOpcUaBaseVariable
{
    Q_OBJECT
public:
    explicit QOpcUaBaseDataVariable(QOpcUaServerNode *parent);

signals:

public slots:
};

#endif // QOPCUABASEDATAVARIABLE_H