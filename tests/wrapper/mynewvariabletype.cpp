#include "mynewvariabletype.h"

MyNewVariableType::MyNewVariableType(QOpcUaServerNode *parent) : QOpcUaBaseDataVariable(parent)
{

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

// NOTE : Forbidden Component
MyNewVariableSubType * MyNewVariableType::getMyProhib()
{
	return m_myProhib;
}


// ---

MyOtherNewVariableType::MyOtherNewVariableType(QOpcUaServerNode *parent) : QOpcUaBaseDataVariable(parent)
{

}

// ---

MyNewVariableSubType::MyNewVariableSubType(QOpcUaServerNode *parent) : MyNewVariableType(parent)
{

}

QOpcUaBaseObject * MyNewVariableSubType::getMyObjSub()
{
	return m_myObjSub;
}

// ---

MyNewVariableSubSubType::MyNewVariableSubSubType(QOpcUaServerNode *parent) : MyNewVariableSubType(parent)
{

}

QOpcUaBaseObject * MyNewVariableSubSubType::getMyObjSubSub()
{
	return m_myObjSubSub;
}
