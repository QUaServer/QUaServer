#ifndef QUAFINITESTATEVARIABLE_H
#define QUAFINITESTATEVARIABLE_H

#include <QUaStateVariable>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.6
/*


*/

class QUaFiniteStateVariable : public QUaStateVariable
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaFiniteStateVariable(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAFINITESTATEVARIABLE_H

