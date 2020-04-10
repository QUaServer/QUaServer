#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include "temperaturesensor.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	// logging

	QObject::connect(&server, &QUaServer::logMessage,
	[](const QUaLog& log) {
		qDebug() << "[" << log.level << "] :" << log.message;
	});

	// register new type

	server.registerType<TemperatureSensor>();

	// create new type instances

	QUaFolderObject * objsFolder = server.objectsFolder();

	objsFolder->addChild<TemperatureSensor>("Sensor1");
	objsFolder->addChild<TemperatureSensor>("Sensor2");
	objsFolder->addChild<TemperatureSensor>("Sensor3");

	server.start();

	return a.exec(); 
}
