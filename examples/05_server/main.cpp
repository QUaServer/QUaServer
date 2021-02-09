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
    [](const QUaLog &log) {
        qDebug() << "[" << log.level << "] :" << log.message;
    });

	// Set custom port
	server.setPort(8080);

	// Load server certificate
	QFile certServer;
	certServer.setFileName("server.crt.der");
	Q_ASSERT(certServer.exists());
	certServer.open(QIODevice::ReadOnly);
	server.setCertificate(certServer.readAll());
	certServer.close();

	// Add server description
	server.setApplicationName ("my_app");
	server.setApplicationUri  ("urn:unconfigured:application"); // NOTE : must match cert info
	server.setProductName     ("my_product");
	server.setProductUri      ("juangburgos.com");
	server.setManufacturerName("My Company Inc.");
	server.setSoftwareVersion ("6.6.6-master");
	server.setBuildNumber     ("gvfsed43fs");

	server.start();

	return a.exec(); 
}
