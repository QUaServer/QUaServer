#ifndef QUALIMITALARM_H
#define QUALIMITALARM_H

#include <QUaAlarmCondition>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 9 - 5.8.11
/*


*/

class QUaLimitAlarm : public QUaAlarmCondition
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaLimitAlarm(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUALIMITALARM_H

