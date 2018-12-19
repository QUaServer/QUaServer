#ifndef QOPCUABASEDATAVARIABLETYPE_H
#define QOPCUABASEDATAVARIABLETYPE_H

#include <QOpcUaBaseVariableType>

class QOpcUaBaseDataVariableType : public QOpcUaBaseVariableType
{
    Q_OBJECT
public:
    explicit QOpcUaBaseDataVariableType(QObject *parent = 0);

    // Node Class - Variable Type - Attributes

signals:

public slots:
};

#endif // QOPCUABASEDATAVARIABLETYPE_H
