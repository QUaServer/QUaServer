#ifndef QUATRANSITIONVARIABLE_H
#define QUATRANSITIONVARIABLE_H

#include <QUaBaseDataVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.4
/*


*/

class QUaTransitionVariable : public QUaBaseDataVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaTransitionVariable(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUATRANSITIONVARIABLE_H

