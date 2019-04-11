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
	
	// Instantiate server by passing certificate contents
	QUaServer server(4840, certServer.readAll());

	// Add server description
	server.setApplicationName ("my_app");
	server.setApplicationUri  ("urn:juangburgos:my_app");
	server.setProductName     ("my_product");
	server.setProductUri      ("juangburgos.com");
	server.setManufacturerName("My Company Inc.");
	server.setSoftwareVersion ("6.6.6-master");
	server.setBuildNumber     ("gvfsed43fs");

	server.start();

	return a.exec(); 
}
