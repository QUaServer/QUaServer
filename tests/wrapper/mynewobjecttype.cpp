#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId)
	: QOpcUaServerNodeFactory<MyNewObjectType>(server, nodeId)
{
	qDebug() << "MyNewObjectType::MyNewObjectType";
}

MyNewVariableSubSubType * MyNewObjectType::getMyVarSubSub()
{
	return this->findChild<MyNewVariableSubSubType*>("myVarSubSub");
}

// ---

MyOtherNewObjectType::MyOtherNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId)
	: QOpcUaServerNodeFactory<MyOtherNewObjectType>(server, nodeId)
{

}
