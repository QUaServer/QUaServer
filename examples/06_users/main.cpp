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
	
	// Logging
	QObject::connect(&server, &QUaServer::logMessage,
		[](const QUaLog& log) {
			qDebug() << "[" << log.level << "] :" << log.message;
		});

	/*
	// It is possible to provide a custom callback to validate the user and password
	// send by the client. The implementation below is the default implementation.
	// Overwriting this callback can be used to obtain a hash of the password sent by the client and
	// compare it to a locally stored hash. In this way we avoid storing user's passwords which
	// is considered an insecure practice. A function pointer can also be used as validation callback.
	server.setUserValidationCallback(
	[&server](const QString &strUserName, const QString &strPassword) {
		if (!server.userExists(strUserName))
		{
			return false;
		}
		QString strKey = server.userKey(strUserName);
		return strKey.compare(strPassword, Qt::CaseSensitive) == 0;
	});
	*/

	// Disable Anon login and create Users
	server.setAnonymousLoginAllowed(false);
	server.addUser("juan", "pass123");
	server.addUser("john", "qwerty");

	QUaFolderObject * objsFolder = server.objectsFolder();

	// Individual variable access control

	// NOTE : the variables need to be overall writable
	//        user-level access is defined later
	auto var1 = objsFolder->addProperty("var1");
	var1->setWriteAccess(true);
	var1->setValue(123);
	auto var2 = objsFolder->addProperty("var2");
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

	auto obj       = objsFolder->addBaseObject("obj");
	auto subobj    = obj->addBaseObject("subobj");
	auto subsubvar = subobj->addProperty("subsubvar");

	subsubvar->setWriteAccess(true);
	subsubvar->setValue("hola");

	// Define access on top level object, 
	// since no specific is defined on 'subsubvar',
	// it inherits the grandparent's
	obj->setUserAccessLevelCallback(&juanCanWrite);

	// Custom types access control

	auto custom1 = objsFolder->addChild<CustomVar>("custom1");
	auto custom2 = objsFolder->addChild<CustomVar>("custom2");

	// Set specific callbacks
	custom1->varFoo()->setUserAccessLevelCallback(&juanCanWrite);
	custom2->setUserAccessLevelCallback(&juanCanWrite);

	// Start server
	server.start();

	return a.exec(); 
}
