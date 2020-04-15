#ifndef QUAEXCLUSIVELIMITSTATEMACHINE_H
#define QUAEXCLUSIVELIMITSTATEMACHINE_H

#include <QUaFiniteStateMachine>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaExclusiveLimitStateMachine : public QUaFiniteStateMachine
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaExclusiveLimitStateMachine(
		QUaServer* server
	);


	// NOTE : the states available for this state machine only
	//        exist on its type. The instance only holds the
	//        current state and last transtion

protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELIMITSTATEMACHINE_H

