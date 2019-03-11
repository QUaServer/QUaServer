#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{
	//qDebug() << "MyNewObjectType::MyNewObjectType";
}

MyNewVariableSubSubType * MyNewObjectType::getMyVarSubSub()
{
	return this->findChild<MyNewVariableSubSubType*>("myVarSubSub");
}

// ---

MyOtherNewObjectType::MyOtherNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{

}
