#include "qopcuaproperty.h"

#include <QOpcUaServer>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaFolderObject>

QOpcUaProperty::QOpcUaProperty(QOpcUaServer *server, const UA_NodeId &nodeId)
{
	this->bindWithUaNode(server, nodeId);
}