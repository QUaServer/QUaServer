#include "qopcuaproperty.h"

#include <QOpcUaServer>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaFolderObject>

QOpcUaProperty::QOpcUaProperty(QOpcUaServer *server, const UA_NodeId &nodeId)
{
	Q_CHECK_PTR(server);
	Q_ASSERT(!UA_NodeId_isNull(&nodeId));
	this->bindWithUaNode(server, nodeId);
}