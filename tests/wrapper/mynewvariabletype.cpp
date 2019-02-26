#include "mynewvariabletype.h"

MyNewVariableType::MyNewVariableType(QOpcUaServerNode *parent) : QOpcUaBaseDataVariable(parent)
{
	
}

MyOtherNewVariableType::MyOtherNewVariableType(QOpcUaServerNode *parent) : QOpcUaBaseDataVariable(parent)
{

}

MyNewVariableSubType::MyNewVariableSubType(QOpcUaServerNode *parent) : MyNewVariableType(parent)
{

}

MyNewVariableSubSubType::MyNewVariableSubSubType(QOpcUaServerNode *parent) : MyNewVariableSubType(parent)
{

}