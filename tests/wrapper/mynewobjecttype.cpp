#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{
	this->myVar()->setValue(0);
}

MyNewVariableSubSubType * MyNewObjectType::myVarSubSub()
{
	return this->findChild<MyNewVariableSubSubType*>("myVarSubSub");
}

QOpcUaBaseDataVariable * MyNewObjectType::myVar()
{
	return this->findChild<QOpcUaBaseDataVariable*>("myVar");
}

// ---

MyOtherNewObjectType::MyOtherNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{

}
