#ifndef QUACONDITIONVARIABLE_H
#define QUACONDITIONVARIABLE_H

#include <QUaBaseDataVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaProperty;


// any change in a ConditionVariable of a Condition should trigger an event
class QUaConditionVariable : public QUaBaseDataVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaConditionVariable(
		QUaServer* server
	);

	// value : data type is BaseDataType (anything)

	void setValue(
		const QVariant        &value, 
		const QUaStatusCode   &statusCode      = QUaStatus::Good,
		const QDateTime       &sourceTimestamp = QDateTime(),
		const QDateTime       &serverTimestamp = QDateTime(),
		const QMetaType::Type &newType         = QMetaType::UnknownType
	) override;

	// children

	// The time of the last change of the Value of this ConditionVariable
	// Overwrite to sync this variable's source timestamp with the child property
	void setSourceTimestamp(const QDateTime& sourceTimestamp) override;

private slots:
	void on_setSourceTimestampChanged(const QDateTime& sourceTimestamp);

private:
	// UtcTime : 
	QUaProperty* getSourceTimestamp();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUACONDITIONVARIABLE_H

