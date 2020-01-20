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

	auto sensor1 = objsFolder->addChild<TemperatureSensor>();
	sensor1->setDisplayName("Sensor1");
	auto sensor2 = objsFolder->addChild<TemperatureSensor>();
	sensor2->setDisplayName("Sensor2");
	auto sensor3 = objsFolder->addChild<TemperatureSensor>();
	sensor3->setDisplayName("Sensor3");

	server.start();

	return a.exec(); 
}
