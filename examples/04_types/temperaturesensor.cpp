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
	return this->browseChild<QUaProperty>("model");
}

QUaProperty * TemperatureSensor::brand()
{
	return this->browseChild<QUaProperty>("brand");
}

QUaProperty * TemperatureSensor::units()
{
	return this->browseChild<QUaProperty>("units");
}

QUaBaseDataVariable * TemperatureSensor::status()
{
	return this->browseChild<QUaBaseDataVariable>("status");
}

QUaBaseDataVariable * TemperatureSensor::currentValue()
{
	return this->browseChild<QUaBaseDataVariable>("currentValue");
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
