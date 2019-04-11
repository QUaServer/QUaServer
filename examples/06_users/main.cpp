#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include "customvar.h"

QUaAccessLevel juanCanWrite(const QString &strUserName) 
{
	QUaAccessLevel access;
	// Read Access to all
	access.bits.bRead = true;
	// Write Access only to juan
	if (strUserName.compare("juan", Qt::CaseSensitive) == 0)
	{
		access.bits.bWrite = true;
	}
	else
	{
		access.bits.bWrite = false;
	}
	return access;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	// Disable Anon login and create Users
	server.setAnonymousLoginAllowed(false);
	server.addUser("juan", "pass123");
	server.addUser("john", "qwerty");

	
	QUaFolderObject * objsFolder = server.objectsFolder();

	// Individual variable access control

	// NOTE : the variables need to be overall writable
	//        user-level access is defined later
	auto var1 = objsFolder->addProperty();
	var1->setDisplayName("var1");
	var1->setWriteAccess(true);
	var1->setValue(123);
	auto var2 = objsFolder->addProperty();
	var2->setDisplayName("var2");
	var2->setWriteAccess(true);
	var2->setValue(1.23);

	// Give access control to variables
	var1->setUserAccessLevelCallback(&juanCanWrite);
	var2->setUserAccessLevelCallback([](const QString &strUserName) {
		QUaAccessLevel access;
		// Read Access to all
		access.bits.bRead = true;
		// Write Access only to john
		if (strUserName.compare("john", Qt::CaseSensitive) == 0)
		{
			access.bits.bWrite = true;
		}
		else
		{
			access.bits.bWrite = false;
		}
		return access;
	});

	// Cascading access control

	auto * obj = objsFolder->addBaseObject();
	obj->setDisplayName("obj");

	auto * subobj = obj->addBaseObject();
	subobj->setDisplayName("subobj");

	auto subsubvar = subobj->addProperty();
	subsubvar->setDisplayName("subsubvar");
	subsubvar->setWriteAccess(true);
	subsubvar->setValue("hola");

	// Define access on top level object, 
	// since no specific is defined on 'subsubvar',
	// it inherits the grandparent's
	obj->setUserAccessLevelCallback(&juanCanWrite);

	// Custom types access control

	auto custom1 = objsFolder->addChild<CustomVar>();
	custom1->setDisplayName("custom1");
	auto custom2 = objsFolder->addChild<CustomVar>();
	custom2->setDisplayName("custom2");

	// Set specific callbacks
	custom1->varFoo()->setUserAccessLevelCallback(&juanCanWrite);
	custom2->setUserAccessLevelCallback(&juanCanWrite);

	// Start server
	server.start();

	return a.exec(); 
}
