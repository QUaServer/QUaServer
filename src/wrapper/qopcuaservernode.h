#ifndef QOPCUASERVERNODE_H
#define QOPCUASERVERNODE_H

#include <QObject>

class QOpcUaServerNode : public QObject
{
    Q_OBJECT
public:
    explicit QOpcUaServerNode(QObject *parent = 0);

signals:

public slots:
};

#endif // QOPCUASERVERNODE_H