#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

#include <QUaServer>

#include "temperaturesensor.h"

#ifdef SQLITE_SERIALIZER
#include "quasqliteserializer.h"
#else
#include "quaxmlserializer.h"
#endif // SQLITE_SERIALIZER

void setupDefaultAddressSpace(QUaServer &server)
{
	QUaFolderObject* objsFolder = server.objectsFolder();

	// create some instances of basic types
	QUaBaseDataVariable* varBaseData = objsFolder->addBaseDataVariable("my_variable");
	varBaseData->setWriteAccess(true);
	varBaseData->setValue(1);
	QUaProperty* varProp = objsFolder->addProperty("my_property", "ns=1;s=my_prop");
	varProp->setValue("hola");
	QUaBaseObject*   objBase    = objsFolder->addBaseObject("my_object", "ns=1;s=my_obj");
	QUaFolderObject* objFolder  = objsFolder->addFolderObject("my_folder");
	QUaProperty*     varSubProp = objBase->addProperty("my_sub_property");
	varSubProp->setValue(666.7);
	QUaBaseObject*   objSubBase    = objBase->addBaseObject("my_sub_object");
	QUaFolderObject* objSubFolder  = objBase->addFolderObject("my_sub_folder");
	QUaBaseObject*   objSubSubBase = objSubFolder->addBaseObject("my_subsub_object", "ns=1;s=my_subsub_object");
	Q_UNUSED(objSubBase);

	// create some instances of custom type
	auto sensor1 = objSubSubBase->addChild<TemperatureSensor>("Sensor1", "ns=1;s=sensor1");
	auto sensor2 = objSubSubBase->addChild<TemperatureSensor>("Sensor2", "ns=1;s=sensor2");
	sensor2->turnOn();
	auto sensor3 = objSubSubBase->addChild<TemperatureSensor>("Sensor3", "ns=1;s=sensor3");
	sensor3->currentValue()->setValue(1.2345);
	// add some non-hierarchical references
	objBase->addReference({ "FriendOf", "FriendOf" }, sensor1);
	sensor2->addReference({ "FriendOf", "FriendOf" }, objFolder);
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	server.registerType<TemperatureSensor>();

	QUaFolderObject* objsFolder = server.objectsFolder();

	QQueue<QUaLog> logOut;
#ifdef SQLITE_SERIALIZER
	const QString strFileName = "config.sqlite";
	QFileInfo fileInfoConf(strFileName);
	bool fileInfoExists = fileInfoConf.exists();
	QUaSqliteSerializer serializer;
	if (!serializer.setSqliteDbName(strFileName, logOut))
#else
	const QString strFileName = "config.xml";
	QFileInfo fileInfoConf(strFileName);
	bool fileInfoExists = fileInfoConf.exists();
	QUaXmlSerializer serializer;
	if (!serializer.setXmlFileName(strFileName, logOut))
#endif // SQLITE_SERIALIZER
	{
		for (auto log : logOut)
		{
			qCritical() << "[" << log.level << "] :" << log.message;
		}
		return 1;
	}
	// deserialize if config file exists, else serialize
	if (fileInfoExists)
	{
		// deserialize
		if (!objsFolder->deserialize(serializer, logOut))
		{
			// print log entries if any
			for (auto log : logOut)
			{
				qCritical() << "[" << log.level << "] :" << log.message;
			}
			return 1;
		}
        // display success
        qInfo() << "[INFO] Deserialized from" << strFileName;
	}
	else
	{
		// create some objects and variables to test
		setupDefaultAddressSpace(server);
		// serialize
		if (!objsFolder->serialize(serializer, logOut))
		{
			for (auto log : logOut)
			{
				qCritical() << "[" << log.level << "] :" << log.message;
			}
			return 1;
		}
        // display success
        qInfo() << "[INFO] Serialized to" << strFileName;
	}

	// print server log
	QObject::connect(&server, &QUaServer::logMessage,
	[](const QUaLog& log) {
		qDebug() << "[" << log.level << "] :" << log.message;
	});
	// print serialization log entries if any
	for (auto log : logOut)
	{
		qDebug() << "[" << log.level << "] :" << log.message;
	}
	// run server
	server.start();

	return a.exec(); 
}
