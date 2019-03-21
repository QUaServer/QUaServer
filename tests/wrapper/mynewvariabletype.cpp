#include "mynewvariabletype.h"

MyNewVariableType::MyNewVariableType(QUaServer *server)
	: QUaBaseDataVariable(server)
{
	this->myVar()->setValue(123);
}

QUaBaseDataVariable * MyNewVariableType::myVar()
{
	return this->findChild<QUaBaseDataVariable*>("myVar");
}

QUaBaseObject * MyNewVariableType::myObj()
{
	return this->findChild<QUaBaseObject*>("myObj");
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

MyOtherNewVariableType::MyOtherNewVariableType(QUaServer *server)
	: QUaBaseDataVariable(server)
{
	this->setValue("equis");
}

QUaBaseDataVariable * MyOtherNewVariableType::myVarTwo()
{
	return this->findChild<QUaBaseDataVariable*>("myVarTwo");
}

// ---

MyNewVariableSubType::MyNewVariableSubType(QUaServer *server)
	: MyNewVariableType(server)
{

}

QUaBaseObject * MyNewVariableSubType::myObjSub()
{
	return this->findChild<QUaBaseObject*>("myObjSub");
}

// ---

MyNewVariableSubSubType::MyNewVariableSubSubType(QUaServer *server)
	: MyNewVariableSubType(server)
{

}

QUaBaseObject * MyNewVariableSubSubType::myObjSubSub()
{
	return this->findChild<QUaBaseObject*>("myObjSubSub");
}


