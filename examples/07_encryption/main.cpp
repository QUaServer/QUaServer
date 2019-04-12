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

	// Load server private key
	QFile privServer;
	privServer.setFileName("server.key.der");
	Q_ASSERT(privServer.exists());
	privServer.open(QIODevice::ReadOnly);

	// Instantiate server by passing certificate and key
	QUaServer server(4840, certServer.readAll(), privServer.readAll());

	certServer.close();
	privServer.close();

	server.start();

	return a.exec(); 
}
