#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QTimer>

#include <QOpcUaServer>

#include "mynewobjecttype.h"
#include "mynewvariabletype.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QOpcUaServer server;
	auto objsFolder = server.get_objectsFolder();

	// instances

	//auto varBaseData = objsFolder->addBaseDataVariable();
	//varBaseData->set_browseName("QOpcUaBaseDataVariable");
	//varBaseData->set_displayName("QOpcUaBaseDataVariable");

	//auto varProp = objsFolder->addProperty();
	//varProp->set_browseName("QOpcUaProperty");
	//varProp->set_displayName("QOpcUaProperty");

	//auto objBase = objsFolder->addBaseObject();
	//objBase->set_browseName("QOpcUaBaseObject");
	//objBase->set_displayName("QOpcUaBaseObject");

	//auto objFolder = objsFolder->addFolderObject();
	//objFolder->set_browseName("QOpcUaFolderObject");
	//objFolder->set_displayName("QOpcUaFolderObject");

	// methods

	objsFolder->addMethod("method1", []() {
		qDebug() << "method1";
	});
	objsFolder->addMethod("method2", []() {
		qDebug() << "method2";
		return "method2";
	});
	objsFolder->addMethod("method3", [](int x, int y) {
		qDebug() << "method3";
		return (double)x / (double)y;
	});
	objsFolder->addMethod("method4", [](int x, double y, QString str) {
		qDebug() << "method4";
		return QString("%1, %2, %3").arg(x).arg(y).arg(str);
	});

	// custom types

	auto newObjTypeInstance = objsFolder->addChild<MyNewObjectType>();
	newObjTypeInstance->getMyVarSubSub()->set_value("");

	//server.registerType<MyNewVariableSubSubType>();
	//server.registerType<MyOtherNewVariableType>();

	//auto newobjTypeInstance = objsFolder->addChild<MyNewObjectType>();
	//newobjTypeInstance->set_browseName ("MyNewObjectType");
	//newobjTypeInstance->set_displayName("MyNewObjectType");

	//auto otherNewobjTypeInstance = objsFolder->addChild<MyOtherNewObjectType>();
	//otherNewobjTypeInstance->set_browseName("MyOtherNewObjectType");
	//otherNewobjTypeInstance->set_displayName("MyOtherNewObjectType");

	//auto newVarTypeInstance = objsFolder->addChild<MyNewVariableType>();
	//newVarTypeInstance->set_browseName("MyNewVariableType");
	//newVarTypeInstance->set_displayName("MyNewVariableType");
	//newVarTypeInstance->set_value(1.2345);

	//qDebug() << "MyNewVariableType->myVar::value =" << newVarTypeInstance->getMyVar()->get_value();
	//newVarTypeInstance->getMyOtherTwo()->getMyVarTwo()->set_value("foo");
	//newVarTypeInstance->getMyOtherVar()->getMyVarTwo()->set_value("bar");

	//auto newVarTypeInstance2 = objsFolder->addChild<MyNewVariableType>();
	//newVarTypeInstance2->set_browseName("MyNewVariableType2");
	//newVarTypeInstance2->set_displayName("MyNewVariableType2");
	//newVarTypeInstance2->set_value("xxx");

	//auto newVarSubTypeInstance = objsFolder->addChild<MyNewVariableSubType>();
	//newVarSubTypeInstance->set_browseName("MyNewVariableSubType");
	//newVarSubTypeInstance->set_displayName("MyNewVariableSubType");
	//newVarSubTypeInstance->set_value(666);

	//auto newVarSubSubTypeInstance = objsFolder->addChild<MyNewVariableSubSubType>();
	//newVarSubSubTypeInstance->set_browseName("MyNewVariableSubSubType");
	//newVarSubSubTypeInstance->set_displayName("MyNewVariableSubSubType");
	//newVarSubSubTypeInstance->set_value(QDateTime::currentDateTime());	


	//auto children = objsFolder->findChildren<QOpcUaBaseObject>();
	//qDebug() << "Children " << children.count();


	// NOTE : runs in main thread within Qt's event loop
	server.start();

    return a.exec();
}
