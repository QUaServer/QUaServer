#ifndef QOPCUABASEDATAVARIABLE_H
#define QOPCUABASEDATAVARIABLE_H

#include <QObject>

class QOpcUaBaseDataVariable : public QObject
{
    Q_OBJECT
public:
    explicit QOpcUaBaseDataVariable(QObject *parent = 0);

signals:

public slots:
};

#endif // QOPCUABASEDATAVARIABLE_H