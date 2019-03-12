#include "mynewvariabletype.h"

MyNewVariableType::MyNewVariableType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseDataVariable(server, nodeId, metaObject)
{
	this->myVar()->setValue(123);
}

QOpcUaBaseDataVariable * MyNewVariableType::myVar()
{
	return this->findChild<QOpcUaBaseDataVariable*>("myVar");
}

QOpcUaBaseObject * MyNewVariableType::myObj()
{
	return this->findChild<QOpcUaBaseObject*>("myObj");
}

MyOtherNewVariableType * MyNewVariableType::myOtherVar()
{
	return this->findChild<MyOtherNewVariableType*>("myOtherVar");
}

MyOtherNewVariableType * MyNewVariableType::myOtherTwo()
{
	return this->findChild<MyOtherNewVariableType*>("myOtherTwo");
}

// ---

MyOtherNewVariableType::MyOtherNewVariableType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseDataVariable(server, nodeId, metaObject)
{
	this->setValue("equis");
}

QOpcUaBaseDataVariable * MyOtherNewVariableType::myVarTwo()
{
	return this->findChild<QOpcUaBaseDataVariable*>("myVarTwo");
}

// ---

MyNewVariableSubType::MyNewVariableSubType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: MyNewVariableType(server, nodeId, metaObject)
{
	//qDebug() << "MyNewVariableSubType::MyNewVariableSubType";
}

QOpcUaBaseObject * MyNewVariableSubType::myObjSub()
{
	return this->findChild<QOpcUaBaseObject*>("myObjSub");
}

// ---

MyNewVariableSubSubType::MyNewVariableSubSubType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: MyNewVariableSubType(server, nodeId, metaObject)
{
	//qDebug() << "MyNewVariableSubSubType::MyNewVariableSubSubType";
}

QOpcUaBaseObject * MyNewVariableSubSubType::myObjSubSub()
{
	return this->findChild<QOpcUaBaseObject*>("myObjSubSub");
}


