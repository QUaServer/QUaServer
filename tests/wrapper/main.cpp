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

	//// NOTE : *runtime* error in console, seems value attribute dominates
	//// - WriteRequest returned status code BadTypeMismatch
	//// - Only Variables with data type BaseDataType may contain a null (empty) value
	//// * This might not be the case when instantiating the variable
	//varBase1->set_dataType(QMetaType::QString); 
	//// - A Solution is that for tree-like API automatically assign dataType when calling set_value
	////   and for object-oriented API, use static members (Static_polymorphism idiom)

	// NOTE : use QVariant::fromValue to force dataType when there is no specific constructor in
	//        http://doc.qt.io/qt-5/qvariant.html
	varBase1->set_value(QByteArray("abcd1234")); 

	auto folder1 = objsFolder->addFolderObject();
	folder1->set_displayName("MyFolder");
	folder1->set_description("This is my first folder");

	auto objBaseNested1 = folder1->addBaseObject();
	objBaseNested1->set_displayName("MyObject_Nested");
	objBaseNested1->set_description("This is my first object nested within a folder");
	objBaseNested1->set_browseName({ 1, "My Browse Name" });

	server.start();

    return a.exec();
}
