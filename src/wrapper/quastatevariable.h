#ifndef QUASTATEVARIABLE_H
#define QUASTATEVARIABLE_H

#include <QUaBaseDataVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS


class QUaStateVariable : public QUaBaseDataVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaStateVariable(
		QUaServer* server
	);

	// value : data type is BaseDataType (anything, specialized in subtypes)

	// children

	// Uniquely identifies the current state within the StateMachineType. 
	QVariant id() const;
	void setId(const QVariant& id);

	// Uniquely identifies the current state within the StateMachineType.
	// NOTE : optional; not created until one of these methods is called
	QUaQualifiedName name() const;
	void setName(const QUaQualifiedName& name);

	// Uniquely identifies the current state within the StateMachineType.
	// NOTE : optional; not created until one of these methods is called
	quint32 number() const;
	void setNumber(const quint32& number);

	// Name of the current (most specific) (sub-)state of the state machine 
	// NOTE : optional; not created until one of these methods is called
	QUaLocalizedText effectiveDisplayName() const;
	void setEffectiveDisplayName(const QUaLocalizedText& effectiveDisplayName);

protected:
	// BaseDataType
	QUaProperty* getId();
	// QualifiedName
	QUaProperty* getName();
	// UInt32
	QUaProperty* getNumber();
	// LocalizedText
	QUaProperty* getEffectiveDisplayName();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUASTATEVARIABLE_H

