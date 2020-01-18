#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	// logging

	QObject::connect(&server, &QUaServer::logMessage,
    [](const QUaLog &log) {
        qDebug() << "[" << log.level << "] :" << log.message;
    });

	QUaFolderObject * objsFolder = server.objectsFolder();

	// basics

	QUaBaseDataVariable * varBaseData = objsFolder->addBaseDataVariable();
	varBaseData->setWriteAccess(true);
	varBaseData->setDisplayName("my_variable");
	varBaseData->setValue(1);
	QObject::connect(varBaseData, &QUaBaseDataVariable::valueChanged, [](const QVariant &value) {
		qDebug() << "New value :" << value;
	});

	QUaProperty * varProp = objsFolder->addProperty("ns=1;s=my_prop");
	varProp->setDisplayName("my_property");
	varProp->setValue("hola");

	QUaBaseObject * objBase = objsFolder->addBaseObject();
	objBase->setDisplayName("my_object");

	QUaFolderObject * objFolder = objsFolder->addFolderObject();
	objFolder->setDisplayName("my_folder");

	// nested nodes and browsing

	QUaBaseObject * obj = objsFolder->addBaseObject();
	obj->setDisplayName("obj");
	obj->setBrowseName ("obj");
	QUaBaseObject * subobj = obj->addBaseObject();
	subobj->setDisplayName("subobj");
	subobj->setBrowseName ("subobj");
	QUaBaseDataVariable * var = subobj->addBaseDataVariable();
	var->setDisplayName("var");
	var->setBrowseName ("var");
	QUaBaseDataVariable * subvar = var->addBaseDataVariable();
	subvar->setDisplayName("subvar");
	subvar->setBrowseName ("subvar");
	QUaProperty * prop = subvar->addProperty();
	prop->setDisplayName("prop");
	prop->setBrowseName ("prop");
	// browse nested children by passing a list of browse names to the (relative) browsePath method
	Q_ASSERT(obj->browsePath(QStringList() << "subobj") == subobj);
	Q_ASSERT(obj->browsePath(QStringList() << "subobj" << "var") == var);
	Q_ASSERT(obj->browsePath(QStringList() << "subobj" << "var" << "subvar") == subvar);
	Q_ASSERT(obj->browsePath(QStringList() << "subobj" << "var" << "subvar" << "prop") == prop);
	// browse from server by passing the (absolute) browse of the node (starting at ObjectsFolder)
	Q_ASSERT(server.browsePath(subobj->nodeBrowsePath()) == subobj);
	Q_ASSERT(server.browsePath(var   ->nodeBrowsePath()) == var   );
	Q_ASSERT(server.browsePath(subvar->nodeBrowsePath()) == subvar);
	Q_ASSERT(server.browsePath(prop  ->nodeBrowsePath()) == prop  );

	// temperature sensor model

	QUaFolderObject * sensorsFolder = objsFolder->addFolderObject();
	sensorsFolder->setDisplayName("Sensors");

	QUaBaseObject * objSensor1 = sensorsFolder->addBaseObject();
	objSensor1->setDisplayName("TempSensor1");

	QUaProperty * modelProp = objSensor1->addProperty();
	modelProp->setDisplayName("Model");
	modelProp->setValue("LM35");
	QUaProperty * brandProp = objSensor1->addProperty();
	brandProp->setDisplayName("Brand");
	brandProp->setValue("Texas Instruments");
	QUaProperty * euProp = objSensor1->addProperty();
	euProp->setDisplayName("Units");
	euProp->setValue("C");

	QUaBaseDataVariable * valueVar = objSensor1->addBaseDataVariable();
	valueVar->setDisplayName("Current Value");
	valueVar->setValue(36.7);

	server.start();

	return a.exec(); 
}
