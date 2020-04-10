#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int addNumbers(int x, int y)
{
	return x + y;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	server.registerEnum<QUaStatus>();

	// logging

	QObject::connect(&server, &QUaServer::logMessage,
	[](const QUaLog& log) {
		qDebug() << "[" << log.level << "] :" << log.message;
	});

	QUaFolderObject * objsFolder = server.objectsFolder();

	// add a method using callback function

	objsFolder->addMethod("addNumbers", &addNumbers);

	// add methods using lambdas

	auto varNumber = objsFolder->addBaseDataVariable("Number");
	varNumber->setValue(0.0);
	varNumber->setDataType(QMetaType::Double);

	objsFolder->addMethod("incrementNumberBy", 
	[&varNumber](double increment, QUaStatus status) {
		if (!varNumber)
		{
			return false;
		}
		double currentValue = varNumber->value().toDouble();
		double newValue = currentValue + increment;
		varNumber->setValue(
			newValue,
			status
		);
		return true;
	});

	objsFolder->addMethod("deleteNumber", 
	[&varNumber]() {
		if (!varNumber)
		{
			return;
		}
		delete varNumber;
		varNumber = nullptr;
	});

	// use QList<T> or QVector<T> as arguments or return types
	// NOTE : only works with supported types T
	objsFolder->addMethod("addNumbersArray", 
	[](QList<int> listInts) {
		int total = 0;
		for (int i = 0; i < listInts.count(); i++)
		{
			total += listInts.at(i);
		}
		return total;
	});

	objsFolder->addMethod("getNames", 
	[]() {
		return QVector<QString>() << "arturo" << "juan" << "miguel";
	});

	// use methods to dynamically create object instances
	objsFolder->addMethod("addObject", 
	[objsFolder](QString strName) {
		auto obj = objsFolder->browseChild<QUaBaseObject>(strName);
		if (obj)
		{
			return QObject::tr("Error : Object with name %1 already exists.").arg(strName);
		}
		obj = objsFolder->addBaseObject(strName, QString("ns=1;s=%1").arg(strName));
		// create delete method for each newly created object
		obj->addMethod("delete", 
		[obj]() {
			// delete instance
			delete obj;
		});
		return QObject::tr("Success.");
	});

	// temperature sensor model

	QUaFolderObject * sensorsFolder = objsFolder->addFolderObject("Sensors");

	QUaBaseObject * objSensor1 = sensorsFolder->addBaseObject("TempSensor1");

	QUaProperty * modelProp = objSensor1->addProperty("Model");
	modelProp->setValue("LM35");
	QUaProperty * brandProp = objSensor1->addProperty("Brand");
	brandProp->setValue("Texas Instruments");
	QUaProperty * euProp = objSensor1->addProperty("Units");
	euProp->setValue("C");

	QUaBaseDataVariable * valueVar = objSensor1->addBaseDataVariable("Current Value");
	valueVar->setValue(36.7);
	QUaBaseDataVariable * statusVar = objSensor1->addBaseDataVariable("Status");
	statusVar->setValue("On");

	objSensor1->addMethod("Turn On", 
	[&statusVar]() {
		statusVar->setValue("On");
	});
	objSensor1->addMethod("Turn Off", 
	[&statusVar]() {
		statusVar->setValue("Off");
	});

	server.start();

	return a.exec(); 
}
