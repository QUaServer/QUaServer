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

	event->setSourceName("MyApplication");
	event->setMessage("An event occured in my server");
	event->setSeverity(66);

	qDebug() << "eventId    " << event->eventId();
	qDebug() << "eventType  " << event->eventType();
	qDebug() << "sourceNode " << event->sourceNode();
	qDebug() << "sourceName " << event->sourceName();
	qDebug() << "time       " << event->time();
	qDebug() << "receiveTime" << event->receiveTime();
	qDebug() << "localTime  " << event->localTime();
	qDebug() << "message    " << event->message();
	qDebug() << "severity   " << event->severity();

	// start server

	server.start();

    return a.exec(); 
}
