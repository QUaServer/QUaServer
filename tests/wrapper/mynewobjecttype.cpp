#include "mynewobjecttype.h"

MyNewObjectType::MyNewObjectType(QUaServer *server)
	: QUaBaseObject(server)
{
	this->myVar()->setDataType(QMetaType::UInt);
	this->myVar()->setValue(0);
	this->myEnumVariable()->setValue(TestEnum::TRES);
}

QUaBaseDataVariable * MyNewObjectType::myVar()
{
	return this->findChild<QUaBaseDataVariable*>("myVar");
}

MyNewVariableSubSubType * MyNewObjectType::myVarSubSub()
{
	return this->findChild<MyNewVariableSubSubType*>("myVarSubSub");
}

QUaBaseDataVariable * MyNewObjectType::myEnumVariable()
{
	return this->findChild<QUaBaseDataVariable*>("myEnum");
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

MyOtherNewObjectType::MyOtherNewObjectType(QUaServer *server)
	: QUaBaseObject(server)
{

}

// ---

MyNewObjectSubType::MyNewObjectSubType(QUaServer *server)
	: MyNewObjectType(server)
{

}

QUaProperty * MyNewObjectSubType::myProp()
{
	return this->findChild<QUaProperty*>("myProp");
}

QString MyNewObjectSubType::concatArgs(int intNum, double dblNum, QString strName)
{
	return QString("%1, %2, %3").arg(intNum).arg(dblNum).arg(strName);
}
