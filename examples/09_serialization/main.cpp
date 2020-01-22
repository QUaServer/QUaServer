#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include "temperaturesensor.h"

struct XmlSerializer
{
	bool writeInstance(
		const QString& nodeId, 
		const QString& typeName, 
		const QMap<QString, QVariant>& attrs, 
		const QList<QUaForwardReference>& forwardRefs
	)
	{
		qDebug() << "NodeId :" << nodeId;
		qDebug() << "TypeName :" << typeName;
		qDebug() << "Attributes :" << [](const QMap<QString, QVariant>& attrs) {
			QString strAttrs;
			QMap<QString, QVariant>::const_iterator i = attrs.constBegin();
			while (i != attrs.constEnd()) {
				strAttrs += QString("{%1, %2}").arg(i.key()).arg(i.value().toString());
				i++;
			}
			return strAttrs;
		}(attrs);
		qDebug() << "References :" << std::accumulate(forwardRefs.begin(), forwardRefs.end(), QString(), [](const QString &str, QUaForwardReference ref) {
			return str + QString("{%1->%2}").arg(ref.refType.strForwardName).arg(ref.nodeIdTarget);
		});
		qDebug() << "--------------------------------------------------------";
		return true;
	}

	bool readInstance(
		const QString& nodeId, 
		QString& typeName, 
		QMap<QString, QVariant>& attrs, 
		QList<QUaForwardReference>& forwardRefs
	)
	{
		return true;
	}
};

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

	XmlSerializer serializer;
	objsFolder->serialize(serializer);

	/*
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

	*/

	server.start();

	return a.exec(); 
}
