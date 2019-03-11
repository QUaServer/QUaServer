#include "qopcuabasedatavariable.h"

QOpcUaBaseDataVariable::QOpcUaBaseDataVariable(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseVariable(server, nodeId, metaObject)
{

}
