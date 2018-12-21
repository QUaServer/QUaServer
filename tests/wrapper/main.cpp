#include <QCoreApplication>
#include <QDebug>

#include <QOpcUaServer>
#include <QOpcUaFolderObject>
#include <QOpcUaBaseVariable>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QOpcUaServer server;
	auto objsFolder = server.get_objectsFolder();
	auto objBase1   = objsFolder->addBaseObject();

	objBase1->set_displayName("MyObject");
	objBase1->set_description("This is my first object");

	qDebug() << objBase1->get_nodeId();

	server.start();

    return a.exec();
}
