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
	void setHighHighLimitAllowed(const bool& highHighLimitAllowed) override;
	void setHighLimitAllowed    (const bool& highLimitAllowed    ) override;
	void setLowLimitAllowed     (const bool& lowLimitAllowed     ) override;
	void setLowLowLimitAllowed  (const bool& lowLowLimitAllowed  ) override;

signals:
	void exclusiveLimitStateChanged(const QUaExclusiveLimitState& state);

protected:
	// No DataType
	QUaExclusiveLimitStateMachine* getLimitState();

	void setExclusiveLimitState(const QUaExclusiveLimitState& exclusiveLimitState);
	bool isExclusiveLimitStateAllowed(const QUaExclusiveLimitState& exclusiveLimitState);

	void processInputNodeValue(const double& value);
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELIMITALARM_H

