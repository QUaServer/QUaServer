#include "temperaturesensor.h"

TemperatureSensor::TemperatureSensor(QUaServer *server)
	: QUaBaseObject(server)
{
	// set defaults
	model()->setValue("TM35");
	brand()->setValue("Texas Instruments");
	units()->setDataTypeEnum(QMetaEnum::fromType<TemperatureSensor::Units>());
	units()->setValue(Units::C);
	status()->setValue("Off");
	currentValue()->setValue(0.0);
	currentValue()->setDataType(QMetaType::Double);
}

QUaProperty * TemperatureSensor::model()
{
	return this->findChild<QUaProperty*>("model");
}

QUaProperty * TemperatureSensor::brand()
{
	return this->findChild<QUaProperty*>("brand");
}

QUaProperty * TemperatureSensor::units()
{
	return this->findChild<QUaProperty*>("units");
}

QUaBaseDataVariable * TemperatureSensor::status()
{
	return this->findChild<QUaBaseDataVariable*>("status");
}

QUaBaseDataVariable * TemperatureSensor::currentValue()
{
	return this->findChild<QUaBaseDataVariable*>("currentValue");
}

void TemperatureSensor::turnOn()
{
	status()->setValue("On");
}

void TemperatureSensor::turnOff()
{
	status()->setValue("Off");
}

void TemperatureSensor::remove()
{
	this->deleteLater();
}
