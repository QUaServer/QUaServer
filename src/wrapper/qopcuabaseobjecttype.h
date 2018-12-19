#ifndef QOPCUABASEOBJECTTYPE_H
#define QOPCUABASEOBJECTTYPE_H

#include <QOpcUaServerNode>

class QOpcUaBaseObjectType : public QOpcUaServerNode
{
    Q_OBJECT
public:
    explicit QOpcUaBaseObjectType(QObject *parent = 0);

signals:

public slots:
};

#endif // QOPCUABASEOBJECTTYPE_H