#ifndef QUATRANSITION_H
#define QUATRANSITION_H

#include <QUaBaseObject>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.10 
/*
Transitions of a FiniteStateMachine are represented as Objects of the ObjectType
TransitionType.

Each valid Transition shall have exactly one FromState Reference and exactly one ToState
Reference, each pointing to an Object of the ObjectType StateType.

Each Transition can have one or more HasCause References pointing to the cause that triggers
the Transition.

Each Transition can have one or more HasEffect References pointing to the effects that occur
when the Transition was triggered.

HasProperty | Variable | TransitionNumber | UInt32 | PropertyType | Mandatory

*/

class QUaTransition : public QUaBaseObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaTransition(
		QUaServer* server
	);

	quint32 transitionNumber() const;
	void    setTransitionNumber(const quint32& transitionNumber);

protected:
	QUaProperty* getTransitionNumber();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUATRANSITION_H

