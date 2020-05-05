#ifndef QUAOFFNORMALALARM_H
#define QUAOFFNORMALALARM_H

#include <QUaDiscreteAlarm>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaProperty;

class QUaOffNormalAlarm : public QUaDiscreteAlarm
{
    Q_OBJECT

	Q_PROPERTY(QVariant normalValue READ normalValue WRITE setNormalValue)

public:
	Q_INVOKABLE explicit QUaOffNormalAlarm(
		QUaServer* server
	);

	// overwrite to subscribe to changes
	virtual void setInputNode(QUaBaseVariable* inputNode) override;

	QVariant normalValue() const;
	void setNormalValue(const QVariant& normalValue);

	// Points to a variable which has a value that is considered to be the normal state
	// of the variable that owns this alarm (see inherited InputNode)
	// NOTE : for now is only informational, logic is implemented using ::setNormalValue
	QUaNodeId normalState() const;
	void setNormalState(const QUaNodeId& normalState);

protected:
	QVariant m_normalValue;

	// NodeId
	QUaProperty* getNormalState();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAOFFNORMALALARM_H

