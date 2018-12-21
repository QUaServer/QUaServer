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

	auto varBase1 = objsFolder->addBaseDataVariable();
	varBase1->set_displayName("MyDataVariable");
	varBase1->set_description("This is my first data variable");
	qDebug() << varBase1->get_nodeId();

	auto folder1 = objsFolder->addFolderObject();
	folder1->set_displayName("MyFolder");
	folder1->set_description("This is my first folder");
	qDebug() << folder1->get_nodeId();

	auto objBaseNested1 = folder1->addBaseObject();
	objBaseNested1->set_displayName("MyObject_Nested");
	objBaseNested1->set_description("This is my first object nested within a folder");
	qDebug() << objBaseNested1->get_nodeId();

	server.start();

    return a.exec();
}
