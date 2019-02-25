#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QTimer>

#include <QOpcUaServer>

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
	



	// [NOTE] blocking, TODO : move to thread
	server.start();

    return a.exec();
}
