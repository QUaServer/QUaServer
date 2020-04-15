#ifndef QUAEXCLUSIVELIMITALARM_H
#define QUAEXCLUSIVELIMITALARM_H

#include <QUaLimitAlarm>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaExclusiveLimitStateMachine;

class QUaExclusiveLimitAlarm : public QUaLimitAlarm
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaExclusiveLimitAlarm(
		QUaServer* server
	);

	// NOTE : inherits limits as properties from base type
	//        but state machine is implemented in the
	//        LimitState of this type

	// The currentState of the LimitState should reflect the
	// limit that us violated if the alarm is active, else null

protected:
	// No DataType
	QUaExclusiveLimitStateMachine* getLimitState();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELIMITALARM_H

