#ifndef QUALIMITALARM_H
#define QUALIMITALARM_H

#include <QUaAlarmCondition>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaLimitAlarm : public QUaAlarmCondition
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaLimitAlarm(
		QUaServer* server
	);

	// NOTE : at least one of the optinal limits below is required
	//        "Base" limits are used for servers supporting "AdaptiveAlarming"

	// if this alarm supports highhigh limit
	bool highHighLimitRequired() const;
	virtual
	void setHighHighLimitRequired(const bool& highHighLimitRequired);
	// NOTE: optional, only work if highHighLimitRequired == true 
	double highHighLimit() const;
	void setHighHighLimit(const double& highHighLimit);

	// if this alarm supports high limit
	bool highLimitRequired() const;
	virtual
	void setHighLimitRequired(const bool& highLimitRequired);
	// NOTE: optional, only work if highLimitRequired == true 
	double highLimit() const;
	void setHighLimit(const double& highLimit);

	// if this alarm supports low limit
	bool lowLimitRequired() const;
	virtual
	void setLowLimitRequired(const bool& lowLimitRequired);
	// NOTE: optional, only work if lowLimitRequired == true 
	double lowLimit() const;
	void setLowLimit(const double& lowLimit);

	// if this alarm supports lowlow limit
	bool lowLowLimitRequired() const;
	virtual
	void setLowLowLimitRequired(const bool& lowLowLimitRequired);
	// NOTE: optional, only work if lowLowLimitRequired == true 
	double lowLowLimit() const;
	void setLowLowLimit(const double& lowLowLimit);

	// adaptive alarming
	bool adaptiveAlarmingSupported() const;
	void setAdaptiveAlarmingSupported(const bool& adaptiveAlarmingSupported);

	// NOTE: optional, only work if baseHighHighLimitRequired == true 
	//       and adaptiveAlarmingSupported == true
	double baseHighHighLimit() const;
	void setBaseHighHighLimit(const double& baseHighHighLimit);

	// NOTE: optional, only work if baseHighLimitRequired == true 
	//       and adaptiveAlarmingSupported == true
	double baseHighLimit() const;
	void setBaseHighLimit(const double& baseHighLimit);

	// NOTE: optional, only work if baseLowLimitRequired == true 
	//       and adaptiveAlarmingSupported == true
	double baseLowLimit() const;
	void setBaseLowLimit(const double& baseLowLimit);

	// NOTE: optional, only work if baseLowLowLimitRequired == true 
	//       and adaptiveAlarmingSupported == true
	double baseLowLowLimit() const;
	void setBaseLowLowLimit(const double& baseLowLowLimit);

signals:
	void highHighLimitRequiredChanged();
	void highHighLimitChanged();

	void highLimitRequiredChanged();
	void highLimitChanged();

	void lowLimitRequiredChanged();
	void lowLimitChanged();

	void lowLowLimitRequiredChanged();
	void lowLowLimitChanged();

protected:
	// helpers
	bool baseHighHighLimitRequired() const;
	virtual
	void setBaseHighHighLimitRequired(const bool& baseHighHighLimitRequired);

	// helpers
	bool baseHighLimitRequired() const;
	virtual
	void setBaseHighLimitRequired(const bool& baseHighLimitRequired);

	// helpers
	bool baseLowLimitRequired() const;
	virtual
	void setBaseLowLimitRequired(const bool& baseLowLimitRequired);

	// helpers
	bool baseLowLowLimitRequired() const;
	virtual
	void setBaseLowLowLimitRequired(const bool& baseLowLowLimitRequired);

	// Double
	QUaProperty* getHighHighLimit();
	// Double
	QUaProperty* getHighLimit();
	// Double
	QUaProperty* getLowLimit();
	// Double
	QUaProperty* getLowLowLimit();
	// Double
	QUaProperty* getBaseHighHighLimit();
	// Double
	QUaProperty* getBaseHighLimit();
	// Double
	QUaProperty* getBaseLowLimit();
	// Double
	QUaProperty* getBaseLowLowLimit();

private:
	bool m_highHighLimitRequired;
	bool m_highLimitRequired;
	bool m_lowLimitRequired;
	bool m_lowLowLimitRequired;

	bool m_adaptiveAlarmingSupported;

	bool m_baseHighHighLimitRequired;
	bool m_baseHighLimitRequired;
	bool m_baseLowLimitRequired;
	bool m_baseLowLowLimitRequired;
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUALIMITALARM_H

