#ifndef QUAOFFNORMALALARM_H
#define QUAOFFNORMALALARM_H

#include <QUaDiscreteAlarm>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaProperty;

class QUaOffNormalAlarm : public QUaDiscreteAlarm
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaOffNormalAlarm(
		QUaServer* server
	);

	// Points to a variable which has a value that is considered to be the normal state
	// of the variable that owns this alarm (see inherited InputNode)
	QUaNodeId normalState() const;
	void setNormalState(const QUaNodeId& normalState);

protected:
	// NodeId
	QUaProperty* getNormalState();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAOFFNORMALALARM_H

