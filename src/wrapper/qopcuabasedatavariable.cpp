#include "qopcuabasedatavariable.h"

QOpcUaBaseDataVariable::QOpcUaBaseDataVariable(QOpcUaServer *server, const UA_NodeId &nodeId)
{
	this->bindWithUaNode(server, nodeId);
}
