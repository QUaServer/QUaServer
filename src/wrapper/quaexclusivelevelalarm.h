#ifndef QUAEXCLUSIVELEVELALARM_H
#define QUAEXCLUSIVELEVELALARM_H

#include <QUaExclusiveLimitAlarm>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaExclusiveLevelAlarm : public QUaExclusiveLimitAlarm
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaExclusiveLevelAlarm(
		QUaServer* server
	);

	// nothing to do here

protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELEVELALARM_H

