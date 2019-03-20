#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: QOpcUaBaseObject(server, nodeId, metaObject)
{
	this->myVar()->setDataType(QMetaType::UInt);
	this->myVar()->setValue(0);
	this->myEnumVariable()->setValue(TestEnum::TRES);
}

QOpcUaBaseDataVariable * MyNewObjectType::myVar()
{
	return this->findChild<QOpcUaBaseDataVariable*>("myVar");
}

MyNewVariableSubSubType * MyNewObjectType::myVarSubSub()
{
	return this->findChild<MyNewVariableSubSubType*>("myVarSubSub");
}

QOpcUaBaseDataVariable * MyNewObjectType::myEnumVariable()
{
	return this->findChild<QOpcUaBaseDataVariable*>("myEnum");
}

MyNewObjectType::TestEnum MyNewObjectType::myEnum()
{
	return (MyNewObjectType::TestEnum)this->myEnumVariable()->value().toInt();
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

// ---

MyNewObjectSubType::MyNewObjectSubType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject)
	: MyNewObjectType(server, nodeId, metaObject)
{

}

QOpcUaProperty * MyNewObjectSubType::myProp()
{
	return this->findChild<QOpcUaProperty*>("myProp");
}

QString MyNewObjectSubType::concatArgs(int intNum, double dblNum, QString strName)
{
	return QString("%1, %2, %3").arg(intNum).arg(dblNum).arg(strName);
}
