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

	objsFolder->addMethod("incrementNumberBy", 
	[&varNumber](double increment) {
		if (!varNumber)
		{
			return false;
		}
		double currentValue = varNumber->value().toDouble();
		double newValue = currentValue + increment;
		varNumber->setValue(newValue);
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

	// [TEST]
	// Enable object for events
	objsFolder->setEventNotifierSubscribeToEvents();
	auto evtChange = objsFolder->createEvent<QUaGeneralModelChangeEvent>();
	//auto evtChange = server.createEvent<QUaGeneralModelChangeEvent>();

	// use methods to dynamically create object instances
	objsFolder->addMethod("addObject", 
	[objsFolder, evtChange](QString strName) {
		auto obj = objsFolder->browseChild<QUaBaseObject>(strName);
		if (obj)
		{
			return QObject::tr("Error : Object with name %1 already exists.").arg(strName);
		}
		obj = objsFolder->addBaseObject(QString("ns=1;s=%1").arg(strName));
		// NOTE : setBrowseName to strName below, helps browseChild above find the object
		obj->setBrowseName(strName);
		obj->setDisplayName(strName);

		// [TEST]
		evtChange->setSourceName("FoldersObject");
		evtChange->setMessage("Node has been added");
		evtChange->setTime(QDateTime::currentDateTimeUtc());
		evtChange->setSeverity(500);
		evtChange->setChanges({
			{
				objsFolder->nodeId(),
				objsFolder->typeDefinitionNodeId(),
				QUaChangeVerb::NodeAdded // QUaChangeVerb::ReferenceAdded
			}
		});
		auto changes = evtChange->changes();
		// NOTE : changes array of struct seems to be saved correctly but thing is sent through wireshark
		evtChange->trigger();

		// create delete method for each newly created object
		obj->addMethod("delete", 
		[obj, evtChange]() {

			// [TEST]
			evtChange->trigger();
			/*
			Please take care that the 'Changes' property is filled correctly with reasonable values. 
			*/

			delete obj; 
		});
		return QObject::tr("Success.");
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
