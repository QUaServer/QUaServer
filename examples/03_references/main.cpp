#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// create objects

	QUaBaseObject * objSensor1 = objsFolder->addBaseObject();
	objSensor1->setDisplayName("TempSensor1");

	QUaBaseObject * objSensor2 = objsFolder->addBaseObject();
	objSensor2->setDisplayName("TempSensor2");

	QUaBaseObject * objSupl1 = objsFolder->addBaseObject();
	objSupl1->setDisplayName("Mouser");

	// create references

	server.registerReferenceType({ "Supplies", "IsSuppliedBy" });
	objSupl1->addReference({ "Supplies", "IsSuppliedBy" }, objSensor1);

	server.registerReferenceType({ "Supplies", "IsSuppliedBy" });
	objSensor2->addReference({ "Supplies", "IsSuppliedBy" }, objSupl1, false);

	// browse references

	qDebug() << "Supplier" << objSupl1->displayName() << "supplies :";
	auto listSensors = objSupl1->findReferences<QUaBaseObject>({ "Supplies", "IsSuppliedBy" });
	for (int i = 0; i < listSensors.count(); i++)
	{
		qDebug() << listSensors.at(i)->displayName();
	}

	qDebug() << objSensor1->displayName() << "supplier is: :";
	auto listSuppliers = objSensor1->findReferences<QUaBaseObject>({ "Supplies", "IsSuppliedBy" }, false);
	qDebug() << listSuppliers.first()->displayName();

	server.start();

	return a.exec(); 
}
