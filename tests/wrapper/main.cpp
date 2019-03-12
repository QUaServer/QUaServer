#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QTimer>

#include <QOpcUaServer>

#include "mynewobjecttype.h"
#include "mynewvariabletype.h"

//void printChildren(QOpcUaServerNode * child, const QString strSpacer)
//{
//	Q_CHECK_PTR(child);
//	qDebug() << strSpacer + "-->" << child->displayName();//child->objectName();
//	for (int i = 0; i < child->children().count(); i++)
//	{
//		printChildren(static_cast<QOpcUaServerNode*>(child->children().at(i)), strSpacer + "--");
//	}
//}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QOpcUaServer server;
	auto objsFolder = server.get_objectsFolder();

	// instances

	//auto varBaseData = objsFolder->addBaseDataVariable();
	//varBaseData->setBrowseName("QOpcUaBaseDataVariable");
	//varBaseData->setDisplayName("QOpcUaBaseDataVariable");

	//auto varProp = objsFolder->addProperty();
	//varProp->setBrowseName("QOpcUaProperty");
	//varProp->setDisplayName("QOpcUaProperty");

	//auto objBase = objsFolder->addBaseObject();
	//objBase->setBrowseName("QOpcUaBaseObject");
	//objBase->setDisplayName("QOpcUaBaseObject");

	//auto objFolder = objsFolder->addFolderObject();
	//objFolder->setBrowseName("QOpcUaFolderObject");
	//objFolder->setDisplayName("QOpcUaFolderObject");

	// methods

	//objsFolder->addMethod("method1", []() {
	//	qDebug() << "method1";
	//});
	//objsFolder->addMethod("method2", []() {
	//	qDebug() << "method2";
	//	return "method2";
	//});
	//objsFolder->addMethod("method3", [](int x, int y) {
	//	qDebug() << "method3";
	//	return (double)x / (double)y;
	//});
	//objsFolder->addMethod("method4", [](int x, double y, QString str) {
	//	qDebug() << "method4";
	//	return QString("%1, %2, %3").arg(x).arg(y).arg(str);
	//});

	// custom types

	//server.registerType<MyNewVariableSubSubType>();
	//server.registerType<MyOtherNewVariableType>();

	auto newobjTypeInstance = objsFolder->addChild<MyNewObjectType>();
	newobjTypeInstance->setDisplayName("Test");

	//auto otherNewobjTypeInstance = objsFolder->addChild<MyOtherNewObjectType>();
	//otherNewobjTypeInstance->setBrowseName("MyOtherNewObjectType");
	//otherNewobjTypeInstance->setDisplayName("MyOtherNewObjectType");

	//auto newVarTypeInstance = objsFolder->addChild<MyNewVariableType>();
	//newVarTypeInstance->setBrowseName("MyNewVariableType");
	//newVarTypeInstance->setDisplayName("MyNewVariableType");
	//newVarTypeInstance->setValue(1.2345);

	//newVarTypeInstance->myOtherTwo()->myVarTwo()->setValue("foo");
	//newVarTypeInstance->myOtherVar()->myVarTwo()->setValue("bar");

	//auto newVarTypeInstance2 = objsFolder->addChild<MyNewVariableType>();
	//newVarTypeInstance2->setBrowseName("MyNewVariableType2");
	//newVarTypeInstance2->setDisplayName("MyNewVariableType2");
	//newVarTypeInstance2->setValue("xxx");

	//auto newVarSubTypeInstance = objsFolder->addChild<MyNewVariableSubType>();
	//newVarSubTypeInstance->setBrowseName("MyNewVariableSubType");
	//newVarSubTypeInstance->setDisplayName("MyNewVariableSubType");
	//newVarSubTypeInstance->setValue(666);

	//auto newVarSubSubTypeInstance = objsFolder->addChild<MyNewVariableSubSubType>();
	//newVarSubSubTypeInstance->setBrowseName("MyNewVariableSubSubType");
	//newVarSubSubTypeInstance->setDisplayName("MyNewVariableSubSubType");
	//newVarSubSubTypeInstance->setValue(QDateTime::currentDateTime());	

	// NOTE : runs in main thread within Qt's event loop
	server.start();

    return a.exec();
}
