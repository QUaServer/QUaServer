#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	// create server

	QUaServer server;

	// add some nodes to the objects folder

	QUaFolderObject * objsFolder = server.objectsFolder();

	QUaBaseDataVariable * varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->setDisplayName("my_variable");

	QUaProperty * varProp = objsFolder->addProperty();
	varProp->setDisplayName("my_property");

	QUaBaseObject * objBase = objsFolder->addBaseObject();
	objBase->setDisplayName("my_object");

	QUaFolderObject * objFolder = objsFolder->addFolderObject();
	objFolder->setDisplayName("my_folder");

	// user executable

	objsFolder->addMethod("method", [](QString strName) {
		return "Hola " + strName;
	});

	objsFolder->setUserExecutableCallback([](const QString &strUserName, QUaNode *node) {
		return false;
	});

	// start server

	server.start();

    return a.exec(); 
}
