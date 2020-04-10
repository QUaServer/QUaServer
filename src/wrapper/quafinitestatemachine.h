#ifndef QUAFINITESTATEMACHINE_H
#define QUAFINITESTATEMACHINE_H

#include <QUaStateMachine>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.5 
/*


*/

class QUaFiniteStateMachine : public QUaStateMachine
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteStateMachine(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITESTATEMACHINE_H

