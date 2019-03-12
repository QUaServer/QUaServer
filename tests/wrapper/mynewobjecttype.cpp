#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{
	this->myVar()->setDataType(QMetaType::UInt);
	this->myVar()->setValue(0);
}

MyNewVariableSubSubType * MyNewObjectType::myVarSubSub()
{
	return this->findChild<MyNewVariableSubSubType*>("myVarSubSub");
}

QOpcUaBaseDataVariable * MyNewObjectType::myVar()
{
	return this->findChild<QOpcUaBaseDataVariable*>("myVar");
}

bool MyNewObjectType::updateMyVar(quint32 newVarVal)
{
	if (newVarVal > 100)
	{
		return false;
	}
	this->myVar()->setValue(newVarVal);
	return true;
}

QString MyNewObjectType::saluteName(QString strName)
{
	return "Hello " + strName;
}

double MyNewObjectType::divideNums(int intNum, int intDen)
{
	return (double)intNum/(double)intDen;
}

// ---

MyOtherNewObjectType::MyOtherNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{

}
