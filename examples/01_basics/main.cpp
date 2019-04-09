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

	QUaBaseDataVariable * varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->setDisplayName("my_variable");
	varBaseData->setValue(1);

	QUaProperty * varProp = objsFolder->addProperty();
	varProp->setDisplayName("my_property");
	varProp->setValue("hola");

	QUaBaseObject * objBase = objsFolder->addBaseObject();
	objBase->setDisplayName("my_object");

	QUaFolderObject * objFolder = objsFolder->addFolderObject();
	objFolder->setDisplayName("my_folder");

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

	objsFolder->setEventNotifierSubscribeToEvents();
	objBase->setEventNotifierSubscribeToEvents();
	auto event = objBase->createEvent<MyEventType>();

	event->setSourceName("MyApplication");
	qDebug() << "-----------------------------------------------------------------";
	qDebug() << "eventType  " << event->eventType();
	qDebug() << "sourceName " << event->sourceName();
	qDebug() << "localTime  " << event->localTime();
	qDebug() << "-----------------------------------------------------------------";
	objsFolder->addMethod("triggerEvent", [&event](int num, quint16 severity) {
		event->setMessage(QString("An event occured in my server with number %1").arg(num));
		event->setTime(QDateTime::currentDateTimeUtc());
		event->setSeverity(severity);
		event->trigger();
		qDebug() << "-----------------------------------------------------------------";
		qDebug() << "eventId    " << event->eventId();
		qDebug() << "time       " << event->time();
		qDebug() << "message    " << event->message();
		qDebug() << "sourceNode " << event->sourceNode();
		qDebug() << "receiveTime" << event->receiveTime();
		qDebug() << "severity   " << event->severity();
		qDebug() << "-----------------------------------------------------------------";
	});

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// start server

	server.start();

    return a.exec(); 
}
