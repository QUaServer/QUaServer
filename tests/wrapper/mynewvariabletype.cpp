#include "mynewvariabletype.h"

MyNewVariableType::MyNewVariableType(QOpcUaServer *server)
	: QOpcUaBaseDataVariable(server)
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

MyOtherNewVariableType::MyOtherNewVariableType(QOpcUaServer *server)
	: QOpcUaBaseDataVariable(server)
{
	this->setValue("equis");
}

QOpcUaBaseDataVariable * MyOtherNewVariableType::myVarTwo()
{
	return this->findChild<QOpcUaBaseDataVariable*>("myVarTwo");
}

// ---

MyNewVariableSubType::MyNewVariableSubType(QOpcUaServer *server)
	: MyNewVariableType(server)
{

}

QOpcUaBaseObject * MyNewVariableSubType::myObjSub()
{
	return this->findChild<QOpcUaBaseObject*>("myObjSub");
}

// ---

MyNewVariableSubSubType::MyNewVariableSubSubType(QOpcUaServer *server)
	: MyNewVariableSubType(server)
{

}

QOpcUaBaseObject * MyNewVariableSubSubType::myObjSubSub()
{
	return this->findChild<QOpcUaBaseObject*>("myObjSubSub");
}


