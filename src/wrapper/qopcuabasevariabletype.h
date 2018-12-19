#ifndef QOPCUABASEVARIABLETYPE_H
#define QOPCUABASEVARIABLETYPE_H

#include <QOpcUaServerNode>

class QOpcUaBaseVariableType : public QOpcUaServerNode
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariableType(QObject *parent = 0);

signals:

public slots:
};

#endif // QOPCUABASEVARIABLETYPE_H
