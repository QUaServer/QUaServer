#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <QUaServer>

#include "temperaturesensor.h"
#include "quaxmlserializer.h"


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaDataType type = QString("Double");
	Q_ASSERT(type == QMetaType::Double);

	QUaDataType type2 = QString("QString");
	Q_ASSERT(type2 == QMetaType::QString);

	QUaServer server;

	QUaFolderObject* objsFolder = server.objectsFolder();

	// basics

	QUaBaseDataVariable* varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->setWriteAccess(true);
	varBaseData->setBrowseName("my_variable");
	varBaseData->setDisplayName("my_variable");
	varBaseData->setValue(1);

	QUaProperty* varProp = objsFolder->addProperty("ns=1;s=my_prop");
	varProp->setBrowseName("my_property");
	varProp->setDisplayName("my_property");
	varProp->setValue("hola");

	QUaBaseObject* objBase = objsFolder->addBaseObject();
	objBase->setBrowseName("my_object");
	objBase->setDisplayName("my_object");

	QUaFolderObject* objFolder = objsFolder->addFolderObject();
	objFolder->setBrowseName("my_folder");
	objFolder->setDisplayName("my_folder");

	QUaProperty* varSubProp = objBase->addProperty();
	varSubProp->setBrowseName("my_sub_property");
	varSubProp->setDisplayName("my_sub_property");
	varSubProp->setValue(666.7);

	QUaBaseObject* objSubBase = objBase->addBaseObject();
	objSubBase->setBrowseName("my_sub_object");
	objSubBase->setDisplayName("my_sub_object");

	QUaFolderObject* objSubFolder = objBase->addFolderObject();
	objSubFolder->setBrowseName("my_sub_folder");
	objSubFolder->setDisplayName("my_sub_folder");

	QUaBaseObject* objSubSubBase = objSubFolder->addBaseObject("ns=1;s=my_subsub_object");
	objSubSubBase->setBrowseName("my_subsub_object");
	objSubSubBase->setDisplayName("my_subsub_object");

	// register new type

	server.registerType<TemperatureSensor>();

	auto sensor1 = objSubSubBase->addChild<TemperatureSensor>();
	sensor1->setDisplayName("Sensor1");
	sensor1->setBrowseName("Sensor1");
	auto sensor2 = objSubSubBase->addChild<TemperatureSensor>();
	sensor2->setDisplayName("Sensor2");
	sensor2->setBrowseName("Sensor2");
	auto sensor3 = objSubSubBase->addChild<TemperatureSensor>();
	sensor3->setDisplayName("Sensor3");
	sensor3->setBrowseName("Sensor3");

	// serialize

	QUaXmlSerializer serializer;
	objsFolder->serialize(serializer);

	// save to file
	QFile fileConfig("config.xml");
	if (fileConfig.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate))
	{
		// create stream
		QTextStream streamConfig(&fileConfig);
		// save config in file
		auto b = serializer.toByteArray();
		streamConfig << b;
	}
	// close files
	fileConfig.close();

	// logging

	QObject::connect(&server, &QUaServer::logMessage,
	[](const QUaLog& log) {
		qDebug() << "[" << log.level << "] :" << log.message;
	});

	//server.start();

	return a.exec(); 
}
