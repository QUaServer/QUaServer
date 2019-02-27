#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QOpcUaServerNode *parent) : QOpcUaBaseObject(parent)
{
	
}

MyNewVariableSubSubType * MyNewObjectType::getMyVarSubSub()
{
	return m_myVarSubSub;
}

// ---

MyOtherNewObjectType::MyOtherNewObjectType(QOpcUaServerNode *parent) : QOpcUaBaseObject(parent)
{

}
