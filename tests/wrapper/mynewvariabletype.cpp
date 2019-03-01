#include "mynewvariabletype.h"

MyNewVariableType::MyNewVariableType(QOpcUaServer *server, const UA_NodeId &nodeId) 
	: QOpcUaServerNodeFactory<MyNewVariableType>(server, nodeId)
{
	qDebug() << "MyNewVariableType C++ constructor.";
}

QOpcUaBaseDataVariable * MyNewVariableType::getMyVar()
{
	return m_myVar;
}

QOpcUaBaseObject * MyNewVariableType::getMyObj()
{
	return m_myObj;
}

MyOtherNewVariableType * MyNewVariableType::getMyOtherVar()
{
	return m_myOtherVar;
}


// ---

MyOtherNewVariableType::MyOtherNewVariableType(QOpcUaServer *server, const UA_NodeId &nodeId)
	: QOpcUaServerNodeFactory<MyOtherNewVariableType>(server, nodeId)
{
	qDebug() << "MyOtherNewVariableType C++ constructor.";
}

// ---

MyNewVariableSubType::MyNewVariableSubType(QOpcUaServer *server, const UA_NodeId &nodeId)
	: QOpcUaServerNodeFactory<MyNewVariableSubType>(server, nodeId)
{

}

QOpcUaBaseObject * MyNewVariableSubType::getMyObjSub()
{
	return m_myObjSub;
}

// ---

MyNewVariableSubSubType::MyNewVariableSubSubType(QOpcUaServer *server, const UA_NodeId &nodeId)
	: QOpcUaServerNodeFactory<MyNewVariableSubSubType>(server, nodeId)
{
	qDebug() << "MyNewVariableSubSubType C++ constructor.";
}

QOpcUaBaseObject * MyNewVariableSubSubType::getMyObjSubSub()
{
	return m_myObjSubSub;
}
