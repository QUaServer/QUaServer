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

	// NOTE: optional, only work if highHighLimitAllowed == true 
	double highHighLimit() const;
	void setHighHighLimit(const double& highHighLimit);

	// NOTE: optional, only work if highLimitAllowed == true 
	double highLimit() const;
	void setHighLimit(const double& highLimit);

	// NOTE: optional, only work if lowLimitAllowed == true 
	double lowLimit() const;
	void setLowLimit(const double& lowLimit);

	// NOTE: optional, only work if lowLowLimitAllowed == true 
	double lowLowLimit() const;
	void setLowLowLimit(const double& lowLowLimit);

	// NOTE: optional, only work if baseHighHighLimitAllowed == true 
	double baseHighHighLimit() const;
	void setBaseHighHighLimit(const double& baseHighHighLimit);

	// NOTE: optional, only work if baseHighLimitAllowed == true 
	double baseHighLimit() const;
	void setBaseHighLimit(const double& baseHighLimit);

	// NOTE: optional, only work if baseLowLimitAllowed == true 
	double baseLowLimit() const;
	void setBaseLowLimit(const double& baseLowLimit);

	// NOTE: optional, only work if baseLowLowLimitAllowed == true 
	double baseLowLowLimit() const;
	void setBaseLowLowLimit(const double& baseLowLowLimit);

	// helpers

	bool highHighLimitAllowed() const;
	virtual
	void setHighHighLimitAllowed(const bool& highHighLimitAllowed);

	bool highLimitAllowed() const;
	virtual
	void setHighLimitAllowed(const bool& highLimitAllowed);

	bool lowLimitAllowed() const;
	virtual
	void setLowLimitAllowed(const bool& lowLimitAllowed);

	bool lowLowLimitAllowed() const;
	virtual
	void setLowLowLimitAllowed(const bool& lowLowLimitAllowed);

	bool baseHighHighLimitAllowed() const;
	virtual
	void setBaseHighHighLimitAllowed(const bool& baseHighHighLimitAllowed);

	bool baseHighLimitAllowed() const;
	virtual
	void setBaseHighLimitAllowed(const bool& baseHighLimitAllowed);

	bool baseLowLimitAllowed() const;
	virtual
	void setBaseLowLimitAllowed(const bool& baseLowLimitAllowed);

	bool baseLowLowLimitAllowed() const;
	virtual
	void setBaseLowLowLimitAllowed(const bool& baseLowLowLimitAllowed);

protected:
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
	bool m_highHighLimitAllowed;
	bool m_highLimitAllowed;
	bool m_lowLimitAllowed;
	bool m_lowLowLimitAllowed;
	bool m_baseHighHighLimitAllowed;
	bool m_baseHighLimitAllowed;
	bool m_baseLowLimitAllowed;
	bool m_baseLowLowLimitAllowed;
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUALIMITALARM_H

