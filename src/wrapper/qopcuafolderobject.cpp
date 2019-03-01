#include "qopcuafolderobject.h"

QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServer *server, const UA_NodeId &nodeId)
{
	this->bindWithUaNode(server, nodeId);
}
