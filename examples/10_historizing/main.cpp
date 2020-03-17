#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject* objsFolder = server.objectsFolder();

	// 

	QUaBaseDataVariable* varBaseData = objsFolder->addBaseDataVariable("ns=1;s=my_variable");
	varBaseData->setDisplayName("my_variable");
	varBaseData->setBrowseName("my_variable");
	varBaseData->setHistorizing(true);
	varBaseData->setReadHistoryAccess(true);
	varBaseData->setWriteAccess(true);
	varBaseData->setValue(1);
	QObject::connect(varBaseData, &QUaBaseDataVariable::valueChanged, [](const QVariant& value) {
		qDebug() << "New value :" << value;
	});

	server.start();

	return a.exec();
}
