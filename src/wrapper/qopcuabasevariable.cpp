#include "qopcuabasevariable.h"

UA_NodeId QOpcUaBaseVariable::m_typeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE);

QOpcUaBaseVariable::QOpcUaBaseVariable(QOpcUaServerNode *parent) : QOpcUaAbstractVariable(parent)
{

}
