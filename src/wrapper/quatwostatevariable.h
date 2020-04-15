#ifndef QUATWOSTATEVARIABLE_H
#define QUATWOSTATEVARIABLE_H

#include <QUaStateVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

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

	QUaLocalizedText currentStateName() const;
	void setCurrentStateName(const QUaLocalizedText& currentStateName);

	// children

	// inherited, but specialized to bool
	bool id();
	void setId(const bool& id);

	// Specifies the time when the current state was entered.
	// NOTE : optional; not created until one of these methods is called
	QDateTime transitionTime() const;
	void setTransitionTime(const QDateTime& transitionTime);

	// Companion for inherited EffectiveDisplayName
	// NOTE : optional; not created until one of these methods is called
	QDateTime effectiveTransitionTime() const;
	void setEffectiveTransitionTime(const QDateTime& effectiveTransitionTime);

	// Name of the state when the Id property is true
	// NOTE : optional; not created until one of these methods is called
	QUaLocalizedText trueState() const;
	void setTrueState(const QUaLocalizedText& trueState);
	// Name of the state when the Id property is false
	// NOTE : optional; not created until one of these methods is called
	QUaLocalizedText falseState() const;
	void setFalseState(const QUaLocalizedText& falseState);

	// TODO . sub states API

	/*
	QList<QUaTwoStateVariable*> browseTrueSubStates() const;

	QUaTwoStateVariable* addTrueSubState(const QUaNodeId& nodeId = QUaNodeId());

	QList<QUaTwoStateVariable*> browseFalseSubStates() const;

	QUaTwoStateVariable* addFalseSubState(const QUaNodeId& nodeId = QUaNodeId());

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

