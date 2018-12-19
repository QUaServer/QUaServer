#ifndef QOPCUAFOLDEROBJECT_H
#define QOPCUAFOLDEROBJECT_H

#include <QOpcUaBaseObject>

class QOpcUaFolderObject : public QOpcUaBaseObject
{
    Q_OBJECT
public:
    explicit QOpcUaFolderObject(QObject *parent = 0);

signals:

public slots:
};

#endif // QOPCUAFOLDEROBJECT_H