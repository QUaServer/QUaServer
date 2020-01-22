#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <QUaServer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// Load server certificate
	QFile certServer;
	certServer.setFileName("server.crt.der");
	Q_ASSERT(certServer.exists());
	certServer.open(QIODevice::ReadOnly);

#ifdef UA_ENABLE_ENCRYPTION
	// Load server private key
	QFile privServer;
	privServer.setFileName("server.key.der");
	Q_ASSERT(privServer.exists());
	privServer.open(QIODevice::ReadOnly);

	// Instantiate server by passing certificate and key
	QUaServer server;
	server.setCertificate(certServer.readAll());
	server.setPrivateKey (privServer.readAll());

	privServer.close();
#else
	QUaServer server;
	server.setCertificate(certServer.readAll());
#endif

	certServer.close();
	
	/*
	Now that communications are encrypted, it is safe to use simple
	username and password aithentication
	*/
	// Disable Anon login and create Users
	server.setAnonymousLoginAllowed(false);
	server.addUser("juan", "pass123");
	server.addUser("john", "qwerty");

	server.start();

	return a.exec(); 
}

