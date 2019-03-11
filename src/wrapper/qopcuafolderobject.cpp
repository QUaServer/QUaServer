#include "qopcuafolderobject.h"

QOpcUaFolderObject::QOpcUaFolderObject(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{

}
