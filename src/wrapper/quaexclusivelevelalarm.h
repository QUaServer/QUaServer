#ifndef QUAEXCLUSIVELEVELALARM_H
#define QUAEXCLUSIVELEVELALARM_H

#include <QUaExclusiveLimitAlarm>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 9 - 5.8.14.3
/*


*/

class QUaExclusiveLevelAlarm : public QUaExclusiveLimitAlarm
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaExclusiveLevelAlarm(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELEVELALARM_H

