#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include <QUaBaseObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

class TemperatureSensor : public QUaBaseObject
{
    Q_OBJECT
	// properties
	Q_PROPERTY(QUaProperty * model READ model)
	Q_PROPERTY(QUaProperty * brand READ brand)
	Q_PROPERTY(QUaProperty * units READ units)
	// variables
	Q_PROPERTY(QUaBaseDataVariable * status       READ status      )
	Q_PROPERTY(QUaBaseDataVariable * currentValue READ currentValue)
public:
	Q_INVOKABLE explicit TemperatureSensor(QUaServer *server);

	enum Units
	{
		C = 0,
		F = 1
	};
	Q_ENUM(Units)
	
	QUaProperty * model();
	QUaProperty * brand();
	QUaProperty * units();

	QUaBaseDataVariable * status      ();
	QUaBaseDataVariable * currentValue();

	Q_INVOKABLE void turnOn();
	Q_INVOKABLE void turnOff();

	Q_INVOKABLE void remove();

};

#endif // TEMPERATURESENSOR_H

