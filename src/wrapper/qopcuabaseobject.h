#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QObject>

class QOpcUaBaseObject : public QObject
{
    Q_OBJECT
public:
    explicit QOpcUaBaseObject(QObject *parent = 0);

signals:

public slots:
};

#endif // QOPCUABASEOBJECT_H