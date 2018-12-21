#include "qopcuabasedatavariable.h"

UA_NodeId QOpcUaNodeFactory<QOpcUaBaseDataVariable>::m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

QOpcUaBaseDataVariable::QOpcUaBaseDataVariable(QOpcUaServerNode *parent) : QOpcUaAbstractVariable(parent)
{

}
