#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId)
	: QOpcUaServerNodeFactory<MyNewObjectType>(server, nodeId)
{
	
}

MyNewVariableSubSubType * MyNewObjectType::getMyVarSubSub()
{
	return m_myVarSubSub;
}

// ---

MyOtherNewObjectType::MyOtherNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId)
	: QOpcUaServerNodeFactory<MyOtherNewObjectType>(server, nodeId)
{

}
