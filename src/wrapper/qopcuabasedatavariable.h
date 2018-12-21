#ifndef QOPCUABASEDATAVARIABLE_H
#define QOPCUABASEDATAVARIABLE_H

#include <QOpcUaAbstractVariable>

class QOpcUaBaseDataVariable : public QOpcUaAbstractVariable, public QOpcUaNodeFactory<QOpcUaBaseDataVariable>
{
    Q_OBJECT
public:
    explicit QOpcUaBaseDataVariable(QOpcUaServerNode *parent);

	static UA_NodeId m_typeNodeId;
};

#endif // QOPCUABASEDATAVARIABLE_H