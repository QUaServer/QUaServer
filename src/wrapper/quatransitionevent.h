#ifndef QUATRANSITIONEVENT_H
#define QUATRANSITIONEVENT_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaTransitionVariable;
class QUaStateVariable;

class QUaTransitionEvent : public QUaBaseEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaTransitionEvent(
		QUaServer *server
	);

	// inherited

	// sourceNode should be NodeId of (Sub-)StateMachine instance

	// The transition that caused the trigger
	QUaLocalizedText transition() const;
	void setTransition(const QUaLocalizedText& transition);

	// The state before the transition
	QUaLocalizedText fromState() const;
	void setFromState(const QUaLocalizedText& fromState);

	// The state after the transition
	QUaLocalizedText toState() const;
	void setToState(const QUaLocalizedText& toState);

protected:
	// LocalizedText
	QUaTransitionVariable* getTransition();
	// LocalizedText
	QUaStateVariable* getFromState();
	// LocalizedText
	QUaStateVariable* getToState();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUATRANSITIONEVENT_H