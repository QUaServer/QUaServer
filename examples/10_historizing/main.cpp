#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>
#include "quainmemoryhistorizer.h"

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	server.start();

	// set historizer (must live at least as long as the server)
	QUaInMemoryHistoryBackend historizer;
	server.setHistorizer(historizer);

	// add test variable
	QUaFolderObject* objsFolder = server.objectsFolder();
	QUaBaseDataVariable* varBaseData = objsFolder->addBaseDataVariable("ns=1;s=my_variable");
	varBaseData->setDisplayName("my_variable");
	varBaseData->setBrowseName("my_variable");
	varBaseData->setHistorizing(true);
	varBaseData->setReadHistoryAccess(true);
	varBaseData->setWriteAccess(true);
	varBaseData->setValue(0);
	QObject::connect(varBaseData, &QUaBaseDataVariable::valueChanged, [](const QVariant& value) {
		qDebug() << "New value :" << value;
	});

	objsFolder->addMethod("add", [varBaseData](int val) {
		varBaseData->setValue(val);
	});

	return a.exec();
}
