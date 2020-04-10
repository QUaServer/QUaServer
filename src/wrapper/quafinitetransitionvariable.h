#ifndef QUAFINITETRANSITIONVARIABLE_H
#define QUAFINITETRANSITIONVARIABLE_H

#include <QUaTransitionVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.7
/*


*/

class QUaFiniteTransitionVariable : public QUaTransitionVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteTransitionVariable(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITETRANSITIONVARIABLE_H

