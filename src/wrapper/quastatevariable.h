#ifndef QUASTATEVARIABLE_H
#define QUASTATEVARIABLE_H

#include <QUaBaseDataVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.3 : StateVariableType
/*
The StateVariableType is the base VariableType for Variables that store the current state of a
StateMachine as a human readable name.

HasSubtype VariableType FiniteStateVariableType Defined in B.4.6
HasProperty | Variable | Id                   | BaseDataType  | PropertyType | Mandatory
HasProperty | Variable | Name                 | QualifiedName | PropertyType | Optional
HasProperty | Variable | Number               | UInt32        | PropertyType | Optional
HasProperty | Variable | EffectiveDisplayName | LocalizedText | PropertyType | Optional

StateMachines produce Events which may include the current state of a StateMachine. In that
case Servers shall provide all the optional Properties of the StateVariableType in the Event,
even if they are not provided on the instances in the AddressSpace.

*/

class QUaStateVariable : public QUaBaseDataVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaStateVariable(
		QUaServer* server
	);

	// value : data type is BaseDataType (anything, specialized in subtypes)

	// children

	// Is a name which uniquely identifies the current state within the StateMachineType. 
	// A subtype may restrict the DataType.
	QVariant id() const;
	void setId(const QVariant& id);

	// Is a QualifiedName which uniquely identifies the current state within the StateMachineType.
	// NOTE : optional; not created until one of these methods is called
	QString name() const;
	void setName(const QString& name);

	// Is an integer which uniquely identifies the current state within the StateMachineType.
	// NOTE : optional; not created until one of these methods is called
	quint32 number() const;
	void setNumber(const quint32& number);

	// Contains a human readable name for the current state of the state
	// machine after taking the state of any SubStateMachines in account.
	// There is no rule specified for which state or sub-state should be used. 
	// It is up to the Server and will depend on the semantics of the StateMachineType.
	// NOTE : optional; not created until one of these methods is called
	QString effectiveDisplayName() const;
	void setEffectiveDisplayName(const QString& effectiveDisplayName);

private:
	// BaseDataType
	QUaProperty* getId();
	// QualifiedName
	QUaProperty* getName();
	// UInt32
	QUaProperty* getNumber();
	// LocalizedText
	QUaProperty* getEffectiveDisplayName();

	// open62541 does not let instantiate if attributes do not match spec
	static UA_VariableAttributes m_vAttr;

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUASTATEVARIABLE_H

