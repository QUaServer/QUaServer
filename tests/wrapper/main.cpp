#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>

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

	auto varBase1 = objsFolder->addBaseDataVariable();
	varBase1->set_displayName("MyDataVariable");
	varBase1->set_description("This is my first data variable");

	//varBase1->set_value(QVariant::fromValue(QList<int>({ 1, 2, 3 })));
	//varBase1->set_value(QVariant::fromValue(QList<int>()));
	//varBase1->set_value(QVariantList() << 1.1 << 2.2 << 3.3);
	//varBase1->set_value(QStringList() << "1.1" << "2.2" << "3.3");
	//varBase1->set_dataType(QMetaType::ULongLong);
	varBase1->set_value(QVariantList() << 1 << 4 << 6);
	//varBase1->set_value(123);
	//
	
	qDebug() << varBase1->get_value();
	
	auto folder1 = objsFolder->addFolderObject();
	folder1->set_displayName("MyFolder");
	folder1->set_description("This is my first folder");

	auto objBaseNested1 = folder1->addBaseObject();
	objBaseNested1->set_displayName("MyObject_Nested");
	objBaseNested1->set_description("This is my first object nested within a folder");
	objBaseNested1->set_browseName({ 1, "My Browse Name" });

	////QVariant testVar = QVariant::fromValue(QList<int>({ 1, 2, 3 }));
	//QVariant testVar = QVariantList({ 1, 2, 3 });
	//if (testVar.canConvert<QVariantList>())
	//{
	//	auto iter = testVar.value<QSequentialIterable>();
	//	auto type = (QMetaType::Type)iter.at(0).type();
	//	qDebug() << "[TYPE]" << type;
	//}


	// [NOTE] blocking
	server.start();

    return a.exec();
}
