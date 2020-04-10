#ifndef QUADISCRETEALARM_H
#define QUADISCRETEALARM_H

#include <QUaAlarmCondition>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 9 - 5.8.17.1 
/*


*/

class QUaDiscreteAlarm : public QUaAlarmCondition
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaDiscreteAlarm(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUADISCRETEALARM_H

