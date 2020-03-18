#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>

#include <QUaServer>
#include "quainmemoryhistorizer.h"

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	QTimer timer;

	QUaServer server;
	server.start();

	// set historizer (must live at least as long as the server)
	QUaInMemoryHistoryBackend historizer;
	server.setHistorizer(historizer);

	// add test variables
	QUaFolderObject* objsFolder = server.objectsFolder();

	for (int i = 0; i < 10; i++)
	{
		// create int variable
		auto varInt = objsFolder->addBaseDataVariable(QString("ns=1;s=Int%1").arg(i));
		varInt->setDisplayName(QString("Int%1").arg(i));
		varInt->setBrowseName(QString("Int%1").arg(i));
		varInt->setHistorizing(true);
		varInt->setReadHistoryAccess(true);
		varInt->setWriteAccess(true);
		varInt->setValue(0);
		// set random value
		QObject::connect(&timer, &QTimer::timeout, varInt, [varInt]() {
			varInt->setValue(QRandomGenerator::global()->generate());
		});
	}

	// update variable every half a second
	timer.start(200);

	return a.exec();
}
