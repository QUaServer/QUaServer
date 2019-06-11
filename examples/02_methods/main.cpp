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

	QUaFolderObject * objsFolder = server.objectsFolder();

	// add a method using callback function

	objsFolder->addMethod("addNumbers", &addNumbers);

	// add methods using lambdas

	auto varNumber = objsFolder->addBaseDataVariable();
	varNumber->setDisplayName("Number");
	varNumber->setValue(0.0);
	varNumber->setDataType(QMetaType::Double);

	objsFolder->addMethod("incrementNumberBy", [&varNumber](double increment) {
		if (!varNumber)
		{
			return false;
		}
		double currentValue = varNumber->value().toDouble();
		double newValue = currentValue + increment;
		varNumber->setValue(newValue);
		return true;
	});

	objsFolder->addMethod("deleteNumber", [&varNumber]() {
		if (!varNumber)
		{
			return;
		}
		delete varNumber;
		varNumber = nullptr;
	});

	// use QList<T> or QVector<T> as arguments or return types
	// NOTE : only works with supported types T

	objsFolder->addMethod("addNumbersArray", [](QList<int> listInts) {
		int total = 0;
		for (int i = 0; i < listInts.count(); i++)
		{
			total += listInts.at(i);
		}
		return total;
	});

	objsFolder->addMethod("getNames", []() {
		return QVector<QString>() << "arturo" << "juan" << "miguel";
	});

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
	QUaBaseDataVariable * statusVar = objSensor1->addBaseDataVariable();
	statusVar->setDisplayName("Status");
	statusVar->setValue("On");

	objSensor1->addMethod("Turn On", [&statusVar]() {
		statusVar->setValue("On");
	});
	objSensor1->addMethod("Turn Off", [&statusVar]() {
		statusVar->setValue("Off");
	});

	server.start();

	return a.exec(); 
}
