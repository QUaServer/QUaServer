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

	auto varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->set_browseName("QOpcUaBaseDataVariable");
	varBaseData->set_displayName("QOpcUaBaseDataVariable");

	auto varProp = objsFolder->addProperty();
	varProp->set_browseName("QOpcUaProperty");
	varProp->set_displayName("QOpcUaProperty");

	auto objBase = objsFolder->addBaseObject();
	objBase->set_browseName("QOpcUaBaseObject");
	objBase->set_displayName("QOpcUaBaseObject");

	auto objFolder = objsFolder->addFolderObject();
	objFolder->set_browseName("QOpcUaFolderObject");
	objFolder->set_displayName("QOpcUaFolderObject");

	// Add new object type

	server.registerType<MyNewVariableSubSubType>();
	server.registerType<MyOtherNewVariableType>();

	auto newobjTypeInstance = objsFolder->addChild<MyNewObjectType>();
	newobjTypeInstance->set_browseName ("MyNewObjectType");
	newobjTypeInstance->set_displayName("MyNewObjectType");

	auto otherNewobjTypeInstance = objsFolder->addChild<MyOtherNewObjectType>();
	otherNewobjTypeInstance->set_browseName("MyOtherNewObjectType");
	otherNewobjTypeInstance->set_displayName("MyOtherNewObjectType");

	auto newVarTypeInstance = objsFolder->addChild<MyNewVariableType>();
	newVarTypeInstance->set_browseName("MyNewVariableType");
	newVarTypeInstance->set_displayName("MyNewVariableType");
	newVarTypeInstance->set_value(1.2345);
	
	auto newVarTypeInstance2 = objsFolder->addChild<MyNewVariableType>();
	newVarTypeInstance2->set_browseName("MyNewVariableType2");
	newVarTypeInstance2->set_displayName("MyNewVariableType2");
	newVarTypeInstance2->set_value("xxx");
	

	// [NOTE] blocking, TODO : move to thread
	server.start();

    return a.exec();
}
