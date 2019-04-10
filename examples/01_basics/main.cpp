#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// basics

	QUaBaseDataVariable * varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->setWriteAccess(true);
	varBaseData->setDisplayName("my_variable");
	varBaseData->setValue(1);
	QObject::connect(varBaseData, &QUaBaseDataVariable::valueChanged, [](const QVariant &value) {
		qDebug() << "New value :" << value;
	});

	QUaProperty * varProp = objsFolder->addProperty("ns=1;s=my_prop");
	varProp->setDisplayName("my_property");
	varProp->setValue("hola");

	QUaBaseObject * objBase = objsFolder->addBaseObject();
	objBase->setDisplayName("my_object");

	QUaFolderObject * objFolder = objsFolder->addFolderObject();
	objFolder->setDisplayName("my_folder");

	// temperature sensor model

	QUaFolderObject * sensorsFolder = objsFolder->addFolderObject();
	sensorsFolder->setDisplayName("Sensors");

	QUaBaseObject * objSensor1 = sensorsFolder->addBaseObject();
	objSensor1->setDisplayName("TempSensor1");

	QUaProperty * modelProp = objSensor1->addProperty();
	modelProp->setDisplayName("Model");
	modelProp->setValue("LM35");
	QUaProperty * brandProp = objSensor1->addProperty();
	brandProp->setDisplayName("Brand");
	brandProp->setValue("Texas Instruments");
	QUaProperty * euProp = objSensor1->addProperty();
	euProp->setDisplayName("Units");
	euProp->setValue("C");

	QUaBaseDataVariable * valueVar = objSensor1->addBaseDataVariable();
	valueVar->setDisplayName("Current Value");
	valueVar->setValue(36.7);

	server.start();

	return a.exec(); 
}
