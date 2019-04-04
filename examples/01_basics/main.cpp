#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	// create server

	QUaServer server;

	server.setApplicationName ("my_app");
	server.setApplicationUri  ("urn:my_product:my_app");
	server.setProductName     ("my_product");
	server.setProductUri      ("my_company.com");
	server.setManufacturerName("My Company Inc.");
	server.setSoftwareVersion ("6.6.6-master");
	server.setBuildNumber     ("gvfsed43fs");

	qDebug() << "applicationName"  << server.applicationName();
	qDebug() << "applicationUri"   << server.applicationUri();
	qDebug() << "productName"      << server.productName();
	qDebug() << "productUri"       << server.productUri();
	qDebug() << "manufacturerName" << server.manufacturerName();
	qDebug() << "softwareVersion"  << server.softwareVersion();
	qDebug() << "buildNumber"      << server.buildNumber();

	// add some nodes to the objects folder

	QUaFolderObject * objsFolder = server.objectsFolder();

	QUaBaseDataVariable * varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->setDisplayName("my_variable");

	QUaProperty * varProp = objsFolder->addProperty();
	varProp->setDisplayName("my_property");

	QUaBaseObject * objBase = objsFolder->addBaseObject();
	objBase->setDisplayName("my_object");

	QUaFolderObject * objFolder = objsFolder->addFolderObject();
	objFolder->setDisplayName("my_folder");

	// start server

	server.start();

    return a.exec(); 
}
