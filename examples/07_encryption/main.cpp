#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <QUaServer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	// Logging
	QObject::connect(&server, &QUaServer::logMessage,
	[](const QUaLog& log) {
 		qDebug() << "[" << log.level << "] :" << log.message;
	});

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
	server.setCertificate(certServer.readAll());
	server.setPrivateKey (privServer.readAll());
	privServer.close();

	// Load client certificate
	QFile certClient;
	certClient.setFileName("client_files/client.crt.der");
	Q_ASSERT(certClient.exists());
	certClient.open(QIODevice::ReadOnly);
	server.addClientCertificate(certClient.readAll());

	// Load certificate authority
	QFile certCertAuth;
	certCertAuth.setFileName("ca_files/ca.crt.der");
	Q_ASSERT(certCertAuth.exists());
	certCertAuth.open(QIODevice::ReadOnly);

	QFile clrCertAuth;
	clrCertAuth.setFileName("ca_files/ca.der.crl");
	Q_ASSERT(clrCertAuth.exists());
	clrCertAuth.open(QIODevice::ReadOnly);

	server.addCertificateAuthority(certCertAuth.readAll(), clrCertAuth.readAll());

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
	// IMPORTANT : the ApplicationUri must match the one used to create
	// the certificate, else the server will fail to start
	server.setApplicationUri("urn:unconfigured:application");

	auto var = server.objectsFolder()->addBaseDataVariable("MyVar", { 0, "MyVar" });
	var->setValue(3.14);
	var->setWriteAccess(true);
	
	server.start();

	return a.exec(); 
}

