#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include "myevent.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	QObject::connect(&server, &QUaServer::logMessage,
    [](const QUaLog &log) {
        qDebug() 
			<< "["   << log.timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
			<< "]["  << log.level
			<< "]["  << log.category
			<< "] :" << log.message;
    });
	QObject::connect(&server, &QUaServer::clientConnected,
	[](const QUaSession * sessionData)
	{
		qDebug() << "Connected" << sessionData->address() << ":" << sessionData->port() << "|" << sessionData->applicationName();
	});

	QUaFolderObject * objsFolder = server.objectsFolder();
	auto obj = objsFolder->addBaseObject();
	obj->setDisplayName("obj");
	obj->setBrowseName ("obj");

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// Create event with server as originator
	auto server_event = server.createEvent<MyEvent>();
	server_event->setDisplayName("MyServerEvent");

	objsFolder->addMethod("triggerServerEvent", [&server_event]() {
		if (!server_event)
		{
			return;
		}
		// Set server_event information
		auto time = QDateTime::currentDateTime();
		server_event->setLocalTime(time.timeZone());
		server_event->setSourceName("Server");
		server_event->setMessage("An event occured in the server");
		server_event->setTime(time.toUTC());
		server_event->setSeverity(100);
		// Trigger server_event
		server_event->trigger();	
	});

	objsFolder->addMethod("deleteServerEvent", [&server_event]() {
		if (!server_event)
		{
			return;
		}
		delete server_event;
		server_event = nullptr;
	});

	// Enable object for events
	obj->setEventNotifierSubscribeToEvents();
	// Create event with object as originator
	auto obj_event = obj->createEvent<MyEvent>();
	obj_event->setDisplayName("MyObjectEvent");

	objsFolder->addMethod("triggerObjectEvent", [&obj_event]() {
		if (!obj_event)
		{
			return;
		}
		// Set server_event information
		obj_event->setSourceName("Object");
		obj_event->setMessage("An event occured in the object");
		obj_event->setTime(QDateTime::currentDateTimeUtc());
		obj_event->setSeverity(300);
		// Trigger server_event
		obj_event->trigger();
	});

	objsFolder->addMethod("deleteObjectEvent", [&obj_event]() {
		if (!obj_event)
		{
			return;
		}
		delete obj_event;
		obj_event = nullptr;
	});

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	server.start();

	return a.exec(); 
}
