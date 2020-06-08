#include "qualimitalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaLimitAlarm::QUaLimitAlarm(
	QUaServer* server
) : QUaAlarmCondition(server)
{
	m_highHighLimitRequired     = false;
	m_highLimitRequired         = false;
	m_lowLimitRequired          = false;
	m_lowLowLimitRequired       = false;
	m_adaptiveAlarmingSupported = false;
	m_baseHighHighLimitRequired  = false;
	m_baseHighLimitRequired      = false;
	m_baseLowLimitRequired       = false;
	m_baseLowLowLimitRequired    = false;
}

double QUaLimitAlarm::highHighLimit() const
{
	Q_ASSERT_X(m_highHighLimitRequired, "QUaLimitAlarm::highHighLimit", "First call setHighHighLimitAllowed");
	if (!m_highHighLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getHighHighLimit()->value<double>();
}

void QUaLimitAlarm::setHighHighLimit(const double& highHighLimit)
{
	Q_ASSERT_X(m_highHighLimitRequired, "QUaLimitAlarm::setHighHighLimit", "First call setHighHighLimitAllowed");
	if (!m_highHighLimitRequired)
	{
		return;
	}
	if (highHighLimit == this->highHighLimit())
	{
		return;
	}
	this->getHighHighLimit()->setValue(highHighLimit);
	emit this->highHighLimitChanged();
}

double QUaLimitAlarm::highLimit() const
{
	Q_ASSERT_X(m_highLimitRequired, "QUaLimitAlarm::highLimit", "First call setHighLimitRequired");
	if (!m_highLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getHighLimit()->value<double>();
}

void QUaLimitAlarm::setHighLimit(const double& highLimit)
{
	Q_ASSERT_X(m_highLimitRequired, "QUaLimitAlarm::setHighLimit", "First call setHighLimitRequired");
	if (!m_highLimitRequired)
	{
		return;
	}
	if (highLimit == this->highLimit())
	{
		return;
	}
	this->getHighLimit()->setValue(highLimit);
	emit this->highLimitChanged();
}

double QUaLimitAlarm::lowLimit() const
{
	Q_ASSERT_X(m_lowLimitRequired, "QUaLimitAlarm::lowLimit", "First call setLowLimitRequired");
	if (!m_lowLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getLowLimit()->value<double>();
}

void QUaLimitAlarm::setLowLimit(const double& lowLimit)
{
	Q_ASSERT_X(m_lowLimitRequired, "QUaLimitAlarm::setLowLimit", "First call setLowLimitRequired");
	if (!m_lowLimitRequired)
	{
		return;
	}
	if (lowLimit == this->lowLimit())
	{
		return;
	}
	this->getLowLimit()->setValue(lowLimit);
	emit this->lowLimitChanged();
}

double QUaLimitAlarm::lowLowLimit() const
{
	Q_ASSERT_X(m_lowLowLimitRequired, "QUaLimitAlarm::lowLowLimit", "First call setLowLowLimitRequired");
	if (!m_lowLowLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getLowLowLimit()->value<double>();
}

void QUaLimitAlarm::setLowLowLimit(const double& lowLowLimit)
{
	Q_ASSERT_X(m_lowLowLimitRequired, "QUaLimitAlarm::setLowLowLimit", "First call setLowLowLimitRequired");
	if (!m_lowLowLimitRequired)
	{
		return;
	}
	if (lowLowLimit == this->lowLowLimit())
	{
		return;
	}
	this->getLowLowLimit()->setValue(lowLowLimit);
	emit this->lowLowLimitChanged();
}

bool QUaLimitAlarm::adaptiveAlarmingSupported() const
{
	return m_adaptiveAlarmingSupported;
}

double QUaLimitAlarm::baseHighHighLimit() const
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseHighHighLimitRequired,
		"QUaLimitAlarm::baseHighHighLimit", "First call setAdaptiveAlarmingSupported and setBaseHighHighLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseHighHighLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getBaseHighHighLimit()->value<double>();
}

void QUaLimitAlarm::setBaseHighHighLimit(const double& baseHighHighLimit)
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseHighHighLimitRequired,
		"QUaLimitAlarm::setBaseHighHighLimit", "First call setAdaptiveAlarmingSupported and setBaseHighHighLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseHighHighLimitRequired)
	{
		return;
	}
	this->getBaseHighHighLimit()->setValue(baseHighHighLimit);
}

double QUaLimitAlarm::baseHighLimit() const
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseHighLimitRequired, 
		"QUaLimitAlarm::baseHighLimit", "First call setAdaptiveAlarmingSupported and setBaseHighLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseHighLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getBaseHighLimit()->value<double>();
}

void QUaLimitAlarm::setBaseHighLimit(const double& baseHighLimit)
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseHighLimitRequired, 
		"QUaLimitAlarm::baseHighLimit", "First call setAdaptiveAlarmingSupported and setBaseHighLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseHighLimitRequired)
	{
		return;
	}
	this->getBaseHighLimit()->setValue(baseHighLimit);
}

double QUaLimitAlarm::baseLowLimit() const
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseLowLimitRequired, 
		"QUaLimitAlarm::baseLowLimit", "First call setAdaptiveAlarmingSupported and setBaseLowLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseLowLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getBaseLowLimit()->value<double>();
}

void QUaLimitAlarm::setBaseLowLimit(const double& baseLowLimit)
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseLowLimitRequired, 
		"QUaLimitAlarm::setBaseLowLimit", "First call setAdaptiveAlarmingSupported and setBaseLowLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseLowLimitRequired)
	{
		return;
	}
	this->getBaseLowLimit()->setValue(baseLowLimit);
}

double QUaLimitAlarm::baseLowLowLimit() const
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseLowLowLimitRequired, 
		"QUaLimitAlarm::baseLowLowLimit", "First call setAdaptiveAlarmingSupported and setBaseLowLowLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseLowLowLimitRequired)
	{
		return 0.0;
	}
	return const_cast<QUaLimitAlarm*>(this)->getBaseLowLowLimit()->value<double>();
}

void QUaLimitAlarm::setBaseLowLowLimit(const double& baseLowLowLimit)
{
	Q_ASSERT_X(m_adaptiveAlarmingSupported && m_baseLowLowLimitRequired, 
		"QUaLimitAlarm::setBaseLowLowLimit", "First call setAdaptiveAlarmingSupported and setBaseLowLowLimitRequired");
	if (!m_adaptiveAlarmingSupported || !m_baseLowLowLimitRequired)
	{
		return;
	}
	this->getBaseLowLowLimit()->setValue(baseLowLowLimit);
}

bool QUaLimitAlarm::highHighLimitRequired() const
{
	return m_highHighLimitRequired;
}

void QUaLimitAlarm::setHighHighLimitRequired(const bool& highHighLimitRequired)
{
	if (highHighLimitRequired == m_highHighLimitRequired)
	{
		return;
	}
	m_highHighLimitRequired = highHighLimitRequired;
	// add or remove component
	auto highHighLimit = this->browseChild<QUaProperty>("HighHighLimit");
	Q_ASSERT(
		(m_highHighLimitRequired && !highHighLimit) ||
		(!m_highHighLimitRequired && highHighLimit)
	);
	if (!m_highHighLimitRequired)
	{
		Q_CHECK_PTR(highHighLimit);
		// remove
		delete highHighLimit;
		return;
	}
	// initialize and set defaults
	highHighLimit = this->browseChild<QUaProperty>("HighHighLimit", true);
	Q_CHECK_PTR(highHighLimit);
	Q_UNUSED(highHighLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	highHighLimit->setValue(+std::numeric_limits<double>::infinity());
	// allow base limits of currently (un)supported limits
	if (m_adaptiveAlarmingSupported)
	{
		this->setBaseHighHighLimitRequired(m_highHighLimitRequired);
	}
	// notify change
	emit this->highHighLimitRequiredChanged();
}

bool QUaLimitAlarm::highLimitRequired() const
{
	return m_highLimitRequired;
}

void QUaLimitAlarm::setHighLimitRequired(const bool& highLimitRequired)
{
	if (highLimitRequired == m_highLimitRequired)
	{
		return;
	}
	m_highLimitRequired = highLimitRequired;
	// add or remove component
	auto highLimit = this->browseChild<QUaProperty>("HighLimit");
	Q_ASSERT(
		(m_highLimitRequired && !highLimit) ||
		(!m_highLimitRequired && highLimit)
	);
	if (!m_highLimitRequired)
	{
		Q_CHECK_PTR(highLimit);
		// remove
		delete highLimit;
		return;
	}
	// initialize and set defaults
	highLimit = this->browseChild<QUaProperty>("HighLimit", true);
	Q_CHECK_PTR(highLimit);
	Q_UNUSED(highLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	highLimit->setValue(+std::numeric_limits<double>::infinity());
	// allow/disallow base limits of currently (un)supported limits
	if (m_adaptiveAlarmingSupported)
	{
		this->setBaseHighLimitRequired(m_highLimitRequired);
	}
	// notify change
	emit this->highLimitRequiredChanged();
}

bool QUaLimitAlarm::lowLimitRequired() const
{
	return m_lowLimitRequired;
}

void QUaLimitAlarm::setLowLimitRequired(const bool& lowLimitRequired)
{
	if (lowLimitRequired == m_lowLimitRequired)
	{
		return;
	}
	m_lowLimitRequired = lowLimitRequired;
	// add or remove component
	auto lowLimit = this->browseChild<QUaProperty>("LowLimit");
	Q_ASSERT(
		(m_lowLimitRequired && !lowLimit) ||
		(!m_lowLimitRequired && lowLimit)
	);
	if (!m_lowLimitRequired)
	{
		Q_CHECK_PTR(lowLimit);
		// remove
		delete lowLimit;
		return;
	}
	// initialize and set defaults
	lowLimit = this->browseChild<QUaProperty>("LowLimit", true);
	Q_CHECK_PTR(lowLimit);
	Q_UNUSED(lowLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	lowLimit->setValue(-std::numeric_limits<double>::infinity());
	// allow/disallow base limits of currently (un)supported limits
	if (m_adaptiveAlarmingSupported)
	{
		this->setBaseLowLimitRequired(m_lowLimitRequired);
	}
	// notify change
	emit this->lowLimitRequiredChanged();
}

bool QUaLimitAlarm::lowLowLimitRequired() const
{
	return m_lowLowLimitRequired;
}

void QUaLimitAlarm::setLowLowLimitRequired(const bool& lowLowLimitRequired)
{
	if (lowLowLimitRequired == m_lowLowLimitRequired)
	{
		return;
	}
	m_lowLowLimitRequired = lowLowLimitRequired;
	// add or remove component
	auto lowLowLimit = this->browseChild<QUaProperty>("LowLowLimit");
	Q_ASSERT(
		(m_lowLowLimitRequired && !lowLowLimit) ||
		(!m_lowLowLimitRequired && lowLowLimit)
	);
	if (!m_lowLowLimitRequired)
	{
		Q_CHECK_PTR(lowLowLimit);
		// remove
		delete lowLowLimit;
		return;
	}
	// initialize and set defaults
	lowLowLimit = this->browseChild<QUaProperty>("LowLowLimit", true);
	Q_CHECK_PTR(lowLowLimit);
	Q_UNUSED(lowLowLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	lowLowLimit->setValue(-std::numeric_limits<double>::infinity());
	// allow/disallow base limits of currently (un)supported limits
	if (m_adaptiveAlarmingSupported)
	{
		this->setBaseLowLowLimitRequired(m_lowLowLimitRequired);
	}
	// notify change
	emit this->lowLowLimitRequiredChanged();
}

bool QUaLimitAlarm::baseHighHighLimitRequired() const
{
	return m_baseHighHighLimitRequired;
}



void QUaLimitAlarm::setAdaptiveAlarmingSupported(const bool& adaptiveAlarmingSupported)
{
	if (adaptiveAlarmingSupported == m_adaptiveAlarmingSupported)
	{
		return;
	}
	m_adaptiveAlarmingSupported = adaptiveAlarmingSupported;
	// allow/disallow base limits of currently (un)supported limits
	if (m_adaptiveAlarmingSupported)
	{
		this->setBaseHighHighLimitRequired(this->highHighLimitRequired());
		this->setBaseHighLimitRequired    (this->highLimitRequired()    );
		this->setBaseLowLimitRequired     (this->lowLimitRequired()     );
		this->setBaseLowLowLimitRequired  (this->lowLowLimitRequired()  );
	}
	else
	{
		this->setBaseHighHighLimitRequired(false);
		this->setBaseHighLimitRequired    (false);
		this->setBaseLowLimitRequired     (false);
		this->setBaseLowLowLimitRequired  (false);
	}
}

void QUaLimitAlarm::setBaseHighHighLimitRequired(const bool& baseHighHighLimitRequired)
{
	if (baseHighHighLimitRequired == m_baseHighHighLimitRequired)
	{
		return;
	}
	m_baseHighHighLimitRequired = baseHighHighLimitRequired;
	// add or remove component
	auto baseHighHighLimit = this->browseChild<QUaProperty>("BaseHighHighLimit");
	Q_ASSERT(
		(m_baseHighHighLimitRequired && !baseHighHighLimit) ||
		(!m_baseHighHighLimitRequired && baseHighHighLimit)
	);
	if (!m_baseHighHighLimitRequired)
	{
		Q_CHECK_PTR(baseHighHighLimit);
		// remove
		delete baseHighHighLimit;
		return;
	}
	// initialize and set defaults
	baseHighHighLimit = this->browseChild<QUaProperty>("BaseHighHighLimit", true);
	Q_CHECK_PTR(baseHighHighLimit);
	Q_UNUSED(baseHighHighLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	baseHighHighLimit->setValue(+std::numeric_limits<double>::infinity());
}

bool QUaLimitAlarm::baseHighLimitRequired() const
{
	return m_baseHighLimitRequired;
}

void QUaLimitAlarm::setBaseHighLimitRequired(const bool& baseHighLimitRequired)
{
	if (baseHighLimitRequired == m_baseHighLimitRequired)
	{
		return;
	}
	m_baseHighLimitRequired = baseHighLimitRequired;
	// add or remove component
	auto baseHighLimit = this->browseChild<QUaProperty>("BaseHighLimit");
	Q_ASSERT(
		(m_baseHighLimitRequired && !baseHighLimit) ||
		(!m_baseHighLimitRequired && baseHighLimit)
	);
	if (!m_baseHighLimitRequired)
	{
		Q_CHECK_PTR(baseHighLimit);
		// remove
		delete baseHighLimit;
		return;
	}
	// initialize and set defaults
	baseHighLimit = this->browseChild<QUaProperty>("BaseHighLimit", true);
	Q_CHECK_PTR(baseHighLimit);
	Q_UNUSED(baseHighLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	baseHighLimit->setValue(+std::numeric_limits<double>::infinity());
}

bool QUaLimitAlarm::baseLowLimitRequired() const
{
	return m_baseLowLimitRequired;
}

void QUaLimitAlarm::setBaseLowLimitRequired(const bool& baseLowLimitRequired)
{
	if (baseLowLimitRequired == m_baseLowLimitRequired)
	{
		return;
	}
	m_baseLowLimitRequired = baseLowLimitRequired;
	// add or remove component
	auto baseLowLimit = this->browseChild<QUaProperty>("BaseLowLimit");
	Q_ASSERT(
		(m_baseLowLimitRequired && !baseLowLimit) ||
		(!m_baseLowLimitRequired && baseLowLimit)
	);
	if (!m_baseLowLimitRequired)
	{
		Q_CHECK_PTR(baseLowLimit);
		// remove
		delete baseLowLimit;
		return;
	}
	// initialize and set defaults
	baseLowLimit = this->browseChild<QUaProperty>("BaseLowLimit", true);
	Q_CHECK_PTR(baseLowLimit);
	Q_UNUSED(baseLowLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	baseLowLimit->setValue(-std::numeric_limits<double>::infinity());
}

bool QUaLimitAlarm::baseLowLowLimitRequired() const
{
	return m_baseLowLowLimitRequired;
}

void QUaLimitAlarm::setBaseLowLowLimitRequired(const bool& baseLowLowLimitRequired)
{
	if (baseLowLowLimitRequired == m_baseLowLowLimitRequired)
	{
		return;
	}
	m_baseLowLowLimitRequired = baseLowLowLimitRequired;
	// add or remove component
	auto baseLowLowLimit = this->browseChild<QUaProperty>("BaseLowLowLimit");
	Q_ASSERT(
		(m_baseLowLowLimitRequired && !baseLowLowLimit) ||
		(!m_baseLowLowLimitRequired && baseLowLowLimit)
	);
	if (!m_baseLowLowLimitRequired)
	{
		Q_CHECK_PTR(baseLowLowLimit);
		// remove
		delete baseLowLowLimit;
		return;
	}
	// initialize and set defaults
	baseLowLowLimit = this->browseChild<QUaProperty>("BaseLowLowLimit", true);
	Q_CHECK_PTR(baseLowLowLimit);
	Q_UNUSED(baseLowLowLimit);
	// NOTE : set default value, no event to avoid recomputing active state
	baseLowLowLimit->setValue(-std::numeric_limits<double>::infinity());
}

QUaProperty* QUaLimitAlarm::getHighHighLimit()
{
	return this->browseChild<QUaProperty>("HighHighLimit");
}

QUaProperty* QUaLimitAlarm::getHighLimit()
{
	return this->browseChild<QUaProperty>("HighLimit");
}

QUaProperty* QUaLimitAlarm::getLowLimit()
{
	return this->browseChild<QUaProperty>("LowLimit");
}

QUaProperty* QUaLimitAlarm::getLowLowLimit()
{
	return this->browseChild<QUaProperty>("LowLowLimit");
}

QUaProperty* QUaLimitAlarm::getBaseHighHighLimit()
{
	return this->browseChild<QUaProperty>("BaseHighHighLimit");
}

QUaProperty* QUaLimitAlarm::getBaseHighLimit()
{
	return this->browseChild<QUaProperty>("BaseHighLimit");
}

QUaProperty* QUaLimitAlarm::getBaseLowLimit()
{
	return this->browseChild<QUaProperty>("BaseLowLimit");
}

QUaProperty* QUaLimitAlarm::getBaseLowLowLimit()
{
	return this->browseChild<QUaProperty>("BaseLowLowLimit");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS


