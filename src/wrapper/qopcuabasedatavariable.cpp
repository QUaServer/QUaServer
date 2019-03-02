#include "qopcuabasedatavariable.h"

QOpcUaBaseDataVariable::QOpcUaBaseDataVariable(QOpcUaServer *server, const UA_NodeId &nodeId)
{
	// check
	if (!server || UA_NodeId_isNull(&nodeId))
	{
		return;
	}
	this->bindWithUaNode(server, nodeId);
}
