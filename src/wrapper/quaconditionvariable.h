#ifndef QUACONDITIONVARIABLE_H
#define QUACONDITIONVARIABLE_H

#include <QUaBaseDataVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaProperty;

// Part 9 - 5.3 : ConditionVariableType
/*
Various information elements of a Condition are not considered to be states. However, a change
in their value is considered important and supposed to trigger an Event Notification. These
information elements are called ConditionVariable.

HasProperty | Variable | SourceTimestamp | UtcTime | PropertyType | Mandatory

*/

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
	// It shall be the same time that would be returned from the Read Service inside the DataValue
	// structure for the ConditionVariable Value Attribute.
	void setSourceTimestamp(const QDateTime& sourceTimestamp) override;

private slots:
	void on_setSourceTimestampChanged(const QDateTime& sourceTimestamp);

private:
	// UtcTime : 
	QUaProperty* getSourceTimestamp();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUACONDITIONVARIABLE_H

