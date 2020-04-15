#ifndef QUATRANSITIONVARIABLE_H
#define QUATRANSITIONVARIABLE_H

#include <QUaBaseDataVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaTransitionVariable : public QUaBaseDataVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaTransitionVariable(
		QUaServer* server
	);

	// inherited

	// TODO : override setValue ?
	// The SourceTimestamp for the value specifies when the Transition occurred.
	// This value may also be exposed with the TransitionTime Property

	// children

	// Uniquely identifies a Transition within the StateMachineType.
	QVariant id() const;
	void setId(const QVariant& id);

	// Name is a QualifiedName which uniquely identifies a transition within the StateMachineType.
	// NOTE : optional; not created until one of these methods is called
	QUaQualifiedName name() const;
	void setName(const QUaQualifiedName& name);

	// Number is an integer which uniquely identifies a transition within the StateMachineType.
	// NOTE : optional; not created until one of these methods is called
	quint32 number() const;
	void setNumber(const quint32& number);

	// TransitionTime specifies when the transition occurred
	// NOTE : optional; not created until one of these methods is called
	QDateTime transitionTime() const;
	void setTransitionTime(const QDateTime& transitionTime);

	// Time when the current (most specific) (sub-)state  entered
	// NOTE : optional; not created until one of these methods is called
	QDateTime effectiveTransitionTime() const;
	void setEffectiveTransitionTime(const QDateTime& effectiveTransitionTime);

protected:
	// BaseDataType
	QUaProperty* getId();
	// QualifiedName
	QUaProperty* getName();
	// UInt32
	QUaProperty* getNumber();
	// UtcTime
	QUaProperty* getTransitionTime();
	// UtcTime
	QUaProperty* getEffectiveTransitionTime();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUATRANSITIONVARIABLE_H

