#ifndef QUATWOSTATEVARIABLE_H
#define QUATWOSTATEVARIABLE_H

#include <QUaStateVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 9 - 5.2 : TwoStateVariableType
/*
Most states defined in this standard are simple – i.e. they are either True or False. The
TwoStateVariableType is introduced specifically for this use case. More complex states are
modelled by using a StateMachineType defined in Part 5.
The TwoStateVariableType is derived from the StateVariableType defined in Part 5 and
formally defined in Table 3(page 14).

HasProperty | Variable | Id                      | Boolean       | PropertyType | Mandatory
HasProperty | Variable | TransitionTime          | UtcTime       | PropertyType | Optional
HasProperty | Variable | EffectiveTransitionTime | UtcTime       | PropertyType | Optional
HasProperty | Variable | TrueState               | LocalizedText | PropertyType | Optional
HasProperty | Variable | FalseState              | LocalizedText | PropertyType | Optional

HasTrueSubState  | StateMachine or TwoStateVariableType | <StateIdentifier> | Defined in Clause 5.4.2 | Optional
HasFalseSubState | StateMachine or TwoStateVariableType | <StateIdentifier> | Defined in Clause 5.4.3 | Optional

The Value Attribute of an instance of TwoStateVariableType contains the current state as a
human readable name. The EnabledState for example, might contain the name “Enabled” when
True and “Disabled” when False.

The optional Property EffectiveDisplayName from the StateVariableType is used if a state has
sub states. It contains a human readable name for the current state after taking the state of any
SubStateMachines in account. As an example, the EffectiveDisplayName of the EnabledState
could contain “Active/HighHigh” to specify that the Condition is active and has exceeded the
HighHigh limit.

Other optional Properties of the StateVariableType have no defined meaning for
TwoStateVariableType.

TrueState and FalseState contain the localized string for the TwoStateVariableType value when
its Id Property has the value True or False, respectively. Since the two Properties provide metadata for the Type, Servers may not allow these Properties to be selected in the Event filter for
a MonitoredItem. Clients can use the Read Service to get the information from the specific
ConditionType.

A HasTrueSubState Reference is used to indicate that the True state has sub states.

A HasFalseSubState Reference is used to indicate that the False state has sub states

*/

class QUaTwoStateVariable : public QUaStateVariable
{
    Q_OBJECT

	// Only mandatory is Id but is already inherited
	//static const QStringList mandatoryChildrenBrowseNames();

public:
	Q_INVOKABLE explicit QUaTwoStateVariable(
		QUaServer* server
	);

	// value : current state as human readable name (maps to value attribute)

	QString currentStateName() const;
	void setCurrentStateName(const QString& currentStateName);

	// children

	// inherited, but specialized to bool
	bool id();
	void setId(const bool& id);

	// Specifies the time when the current state was entered.
	// NOTE : optional; not created until one of these methods is called
	QDateTime transitionTime() const;
	void setTransitionTime(const QDateTime& transitionTime);

	// Specifies the time when the current state or one of its sub states was entered.
	// If, for example, a LevelAlarm is active and – while active – switches several times
	// between High and HighHigh, then the TransitionTime stays at the point in time where 
	// the Alarm became active whereas the EffectiveTransitionTime changes with each 
	// shift of a sub state.
	// Used in conjuction with inherited EffectiveDisplayName which  contains a 
	// human readable name for the current (sub) state, for example "Active/HighHigh"
	// NOTE : optional; not created until one of these methods is called
	QDateTime effectiveTransitionTime() const;
	void setEffectiveTransitionTime(const QDateTime& effectiveTransitionTime);

	// Contain the localized string for the TwoStateVariableType value when
	// its Id Property has the value True
	// NOTE : optional; not created until one of these methods is called
	QString trueState() const;
	void setTrueState(const QString& trueState);
	// Contain the localized string for the TwoStateVariableType value when
	// its Id Property has the value False
	// NOTE : optional; not created until one of these methods is called
	QString falseState() const;
	void setFalseState(const QString& falseState);

	// TODO . sub states API

	/*
	QList<QUaTwoStateVariable*> browseTrueSubStates() const;

	QUaTwoStateVariable* addTrueSubState(const QString& strNodeId = "");

	QList<QUaTwoStateVariable*> browseFalseSubStates() const;

	QUaTwoStateVariable* addFalseSubState(const QString& strNodeId = "");

	// TODO and for StateMachine type
	*/

protected:

	// NOTE : id (Boolean) inherited

	// UtcTime
	QUaProperty* getTransitionTime();
	// UtcTime
	QUaProperty* getEffectiveTransitionTime();
	// LocalizedText
	QUaProperty* getTrueState();
	// LocalizedText
	QUaProperty* getFalseState();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUATWOSTATEVARIABLE_H

