#include "qopcuafolderobject.h"

QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServer *server, const UA_NodeId &nodeId)
{
	// check
	if (!server || UA_NodeId_isNull(&nodeId))
	{
		return;
	}
	this->bindWithUaNode(server, nodeId);
}
