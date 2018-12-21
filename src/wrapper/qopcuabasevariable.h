#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaAbstractVariable>

class QOpcUaBaseVariable : public QOpcUaAbstractVariable, public QOpcUaNodeFactory<QOpcUaBaseVariable>
{
    Q_OBJECT
public:
    explicit QOpcUaBaseVariable(QOpcUaServerNode *parent);

	static UA_NodeId m_typeNodeId;
};

#endif // QOPCUABASEVARIABLE_H