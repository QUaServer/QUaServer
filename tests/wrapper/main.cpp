#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QFile>

#include <QUaServer>

#include "mynewobjecttype.h"
#include "mynewvariabletype.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QFile certServer;
	certServer.setFileName("server.der");
	Q_ASSERT(certServer.exists());
	certServer.open(QIODevice::ReadOnly);

	QUaServer server(4840, certServer.readAll());
	auto objsFolder = server.objectsFolder();

	certServer.close();

/*
	// instances

	auto varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->setBrowseName("QUaBaseDataVariable");
	varBaseData->setDisplayName("QUaBaseDataVariable");

	auto varProp = objsFolder->addProperty();
	varProp->setBrowseName("QUaProperty");
	varProp->setDisplayName("QUaProperty");

	auto objBase = objsFolder->addBaseObject();
	objBase->setBrowseName("QUaBaseObject");
	objBase->setDisplayName("QUaBaseObject");

	auto objFolder = objsFolder->addFolderObject();
	objFolder->setBrowseName("QUaFolderObject");
	objFolder->setDisplayName("QUaFolderObject");

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

	auto newobjTypeInstance = objsFolder->addChild<MyNewObjectSubType>();
	newobjTypeInstance->setDisplayName("Test");

	auto otherNewobjTypeInstance = objsFolder->addChild<MyOtherNewObjectType>();
	otherNewobjTypeInstance->setBrowseName("MyOtherNewObjectType");
	otherNewobjTypeInstance->setDisplayName("MyOtherNewObjectType");

	auto newVarTypeInstance = objsFolder->addChild<MyNewVariableType>();
	newVarTypeInstance->setBrowseName("MyNewVariableType");
	newVarTypeInstance->setDisplayName("MyNewVariableType");
	newVarTypeInstance->setValue(1.2345);

	newVarTypeInstance->myOtherTwo()->myVarTwo()->setValue("foo");
	newVarTypeInstance->myOtherVar()->myVarTwo()->setValue("bar");

	auto newVarTypeInstance2 = objsFolder->addChild<MyNewVariableType>();
	newVarTypeInstance2->setBrowseName("MyNewVariableType2");
	newVarTypeInstance2->setDisplayName("MyNewVariableType2");
	newVarTypeInstance2->setValue("xxx");

	auto newVarSubTypeInstance = objsFolder->addChild<MyNewVariableSubType>();
	newVarSubTypeInstance->setBrowseName("MyNewVariableSubType");
	newVarSubTypeInstance->setDisplayName("MyNewVariableSubType");
	newVarSubTypeInstance->setValue(666);

	auto newVarSubSubTypeInstance = objsFolder->addChild<MyNewVariableSubSubType>();
	newVarSubSubTypeInstance->setBrowseName("MyNewVariableSubSubType");
	newVarSubSubTypeInstance->setDisplayName("MyNewVariableSubSubType");
	newVarSubSubTypeInstance->setValue(QDateTime::currentDateTime());	

	// delete

	objsFolder->addMethod("delete_node", [&newVarTypeInstance]() {
		if (newVarTypeInstance)
		{
			delete newVarTypeInstance;
			newVarTypeInstance = nullptr;
			return "Deleted";
		}
		return "Already Deleted";
	});
*/

	// references

	auto var1 = objsFolder->addBaseDataVariable("ns = 1 ; s = var1");
	var1->setDisplayName("var1");
	var1->setValue(1);
	objsFolder->addMethod("deleteVar1", [&var1]() {
		if (var1) { delete var1; var1 = nullptr; }
	});
	auto var2 = objsFolder->addBaseDataVariable("ns=1;s=var2");
	var2->setDisplayName("var2");
	var2->setValue(2);
	objsFolder->addMethod("deleteVar2", [&var2]() {
		if (var2) { delete var2; var2 = nullptr; }
	});
	auto obj1 = objsFolder->addBaseObject("ns=1;s=obj1");
	obj1->setDisplayName("obj1");

	// obj1 "hasVar" var1
	obj1->addReference({ "hasVar", "isVarOf" }, var1);
	// var2 "isVarOf" obj1
	var2->addReference({ "hasVar", "isVarOf" }, obj1, false);

	auto var1ref = obj1->getReferences<QUaBaseDataVariable>({ "hasVar", "isVarOf" }).first();
	Q_ASSERT(*var1 == *var1ref);

	auto obj1ref = server.getNodebyId<QUaBaseObject>("ns=1;s=obj1");
	Q_ASSERT(*obj1 == *obj1ref);

	obj1->addMethod("addReferences", [&obj1, &var1, &var2]() {
		if(var1) obj1->addReference({ "hasVar", "isVarOf" }, var1);
		if(var2) var2->addReference({ "hasVar", "isVarOf" }, obj1, false);
	});

	obj1->addMethod("removeReferences", [&obj1, &var1, &var2]() {
		if (var1) obj1->removeReference({ "hasVar", "isVarOf" }, var1);
		if (var2) var2->removeReference({ "hasVar", "isVarOf" }, obj1, false);
	});

	// NOTE : runs in main thread within Qt's event loop
	server.start();

    return a.exec(); 
}
