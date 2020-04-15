#ifndef QUAFINITESTATEMACHINE_H
#define QUAFINITESTATEMACHINE_H

#include <QUaStateMachine>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaBaseDataVariable;

// For finite state machines all its possible states are explicitly specified by its type definition
class QUaFiniteStateMachine : public QUaStateMachine
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteStateMachine(
		QUaServer* server
	);

	// inherited

	// QUaLocalizedText currentState

	// QUaLocalizedText lastTransition

	// children

	// NOTE : optional; not created until one of these methods is called
	QList<QUaNodeId> availableStates() const;
	void setAvailableStates(const QList<QUaNodeId>& availableStates);

	// NOTE : optional; not created until one of these methods is called
	QList<QUaNodeId> availableTransitions() const;
	void setAvailableTransitions(const QList<QUaNodeId>& availableTransitions);

protected:
	QUaBaseDataVariable* getAvailableStates();

	QUaBaseDataVariable* getAvailableTransitions();
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITESTATEMACHINE_H

