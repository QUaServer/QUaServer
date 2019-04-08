#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include "myeventtype.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	// create server

	QUaServer server;

	// add some nodes to the objects folder

	QUaFolderObject * objsFolder = server.objectsFolder();

	//QUaBaseDataVariable * varBaseData = objsFolder->addBaseDataVariable();
	//varBaseData->setDisplayName("my_variable");
	//varBaseData->setValue(1);

	//QUaProperty * varProp = objsFolder->addProperty();
	//varProp->setDisplayName("my_property");
	//varProp->setValue("hola");

	//QUaBaseObject * objBase = objsFolder->addBaseObject();
	//objBase->setDisplayName("my_object");

	//QUaFolderObject * objFolder = objsFolder->addFolderObject();
	//objFolder->setDisplayName("my_folder");

	auto event = server.createEvent<MyEventType>();
	qDebug() << event->eventType()->value();

	// start server

	server.start();

    return a.exec(); 
}
