#ifndef QOPCUASERVER_H
#define QOPCUASERVER_H

#include <QObject>

class QOpcUaServer : public QObject
{
    Q_OBJECT
public:
    explicit QOpcUaServer(QObject *parent = 0);

signals:

public slots:
};

#endif // QOPCUASERVER_H