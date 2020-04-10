#ifndef QUAOFFNORMALALARM_H
#define QUAOFFNORMALALARM_H

#include <QUaDiscreteAlarm>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 9 - 5.8.17.2
/*


*/

class QUaOffNormalAlarm : public QUaDiscreteAlarm
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaOffNormalAlarm(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAOFFNORMALALARM_H

