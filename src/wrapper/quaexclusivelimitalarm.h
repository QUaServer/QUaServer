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

	// NOTE : inherits limits as properties from base type but 
	// state machine is implemented in the LimitState of this type
	// The currentState of the LimitState (machine) should reflect the
	// limit that is violated only if the alarm is active, else null

	// overwrite to subscribe to changes
	virtual void setInputNode(QUaBaseVariable* inputNode) override;

	// calls the internal machine method
	QUaExclusiveLimitState exclusiveLimitState() const;

	// NOTE : below specializations call base implementation and update
	//        the available states and transitions of the LimitState machine
	void setHighHighLimitRequired(const bool& highHighLimitRequired) override;
	void setHighLimitRequired    (const bool& highLimitRequired    ) override;
	void setLowLimitRequired     (const bool& lowLimitRequired     ) override;
	void setLowLowLimitRequired  (const bool& lowLowLimitRequired  ) override;

signals:
	void exclusiveLimitStateChanged(const QUaExclusiveLimitState& state);

protected:
	// No DataType
	QUaExclusiveLimitStateMachine* getLimitState();

	void setExclusiveLimitState(const QUaExclusiveLimitState& exclusiveLimitState);
	bool isExclusiveLimitStateAllowed(const QUaExclusiveLimitState& exclusiveLimitState);

	void processInputNodeValue(const double& value);

	void forceActiveStateRecalculation();
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELIMITALARM_H

