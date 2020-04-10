#ifndef QUAEXCLUSIVELIMITALARM_H
#define QUAEXCLUSIVELIMITALARM_H

#include <QUaLimitAlarm>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 9 - 5.8.12.3
/*


*/

class QUaExclusiveLimitAlarm : public QUaLimitAlarm
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaExclusiveLimitAlarm(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELIMITALARM_H

