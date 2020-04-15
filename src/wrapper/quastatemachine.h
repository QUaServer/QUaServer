#ifndef QUASTATEMACHINE_H
#define QUASTATEMACHINE_H

#include <QUaBaseObject>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaStateVariable;
class QUaTransitionVariable;

class QUaStateMachine : public QUaBaseObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaStateMachine(
		QUaServer* server
	);

	// Indicates current state (state instance only exists in type definition)
	QUaLocalizedText currentState() const;
	void setCurrentState(const QUaLocalizedText& currentState);

	// Indicates last transtion (last transtion only exists in type definition)
	// NOTE : optional; not created until one of these methods is called
	QUaLocalizedText lastTransition() const;
	void setLastTransition(const QUaLocalizedText& lastTransition);

protected:
	// LocalizedText
	QUaStateVariable* getCurrentState();
	// LocalizedText
	QUaTransitionVariable* getLastTransition();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUASTATEMACHINE_H

