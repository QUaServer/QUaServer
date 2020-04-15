#include "qualimitalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaLimitAlarm::QUaLimitAlarm(
	QUaServer* server
) : QUaAlarmCondition(server)
{
	m_highHighLimitAllowed     = false;
	m_highLimitAllowed         = false;
	m_lowLimitAllowed          = false;
	m_lowLowLimitAllowed       = false;
	m_baseHighHighLimitAllowed = false;
	m_baseHighLimitAllowed     = false;
	m_baseLowLimitAllowed      = false;
	m_baseLowLowLimitAllowed   = false;
}

double QUaLimitAlarm::highHighLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getHighHighLimit()->value<double>();
}

void QUaLimitAlarm::setHighHighLimit(const double& highHighLimit)
{
	this->getHighHighLimit()->setValue(highHighLimit);
}

double QUaLimitAlarm::highLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getHighLimit()->value<double>();
}

void QUaLimitAlarm::setHighLimit(const double& highLimit)
{
	this->getHighLimit()->setValue(highLimit);
}

double QUaLimitAlarm::lowLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getLowLimit()->value<double>();
}

void QUaLimitAlarm::setLowLimit(const double& lowLimit)
{
	this->getLowLimit()->setValue(lowLimit);
}

double QUaLimitAlarm::lowLowLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getLowLowLimit()->value<double>();
}

void QUaLimitAlarm::setLowLowLimit(const double& lowLowLimit)
{
	this->getLowLowLimit()->setValue(lowLowLimit);
}

double QUaLimitAlarm::baseHighHighLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getBaseHighHighLimit()->value<double>();
}

void QUaLimitAlarm::setBaseHighHighLimit(const double& baseHighHighLimit)
{
	this->getBaseHighHighLimit()->setValue(baseHighHighLimit);
}

double QUaLimitAlarm::baseHighLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getBaseHighLimit()->value<double>();
}

void QUaLimitAlarm::setBaseHighLimit(const double& baseHighLimit)
{
	this->getBaseHighLimit()->setValue(baseHighLimit);
}

double QUaLimitAlarm::baseLowLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getBaseLowLimit()->value<double>();
}

void QUaLimitAlarm::setBaseLowLimit(const double& baseLowLimit)
{
	this->getBaseLowLimit()->setValue(baseLowLimit);
}

double QUaLimitAlarm::baseLowLowLimit() const
{
	return const_cast<QUaLimitAlarm*>(this)->getBaseLowLowLimit()->value<double>();
}

void QUaLimitAlarm::setBaseLowLowLimit(const double& baseLowLowLimit)
{
	this->getBaseLowLowLimit()->setValue(baseLowLowLimit);
}

bool QUaLimitAlarm::highHighLimitAllowed() const
{
	return m_highHighLimitAllowed;
}

void QUaLimitAlarm::setHighHighLimitAllowed(const bool& highHighLimitAllowed)
{
	if (highHighLimitAllowed == m_highHighLimitAllowed)
	{
		return;
	}
	m_highHighLimitAllowed = highHighLimitAllowed;
	// add or remove component
	auto highHighLimit = this->browseChild<QUaProperty>("HighHighLimit");
	Q_ASSERT(
		(m_highHighLimitAllowed && !highHighLimit) ||
		(!m_highHighLimitAllowed && highHighLimit)
	);
	if (!m_highHighLimitAllowed)
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
}

bool QUaLimitAlarm::highLimitAllowed() const
{
	return m_highLimitAllowed;
}

void QUaLimitAlarm::setHighLimitAllowed(const bool& highLimitAllowed)
{
	if (highLimitAllowed == m_highLimitAllowed)
	{
		return;
	}
	m_highLimitAllowed = highLimitAllowed;
	// add or remove component
	auto highLimit = this->browseChild<QUaProperty>("HighLimit");
	Q_ASSERT(
		(m_highLimitAllowed && !highLimit) ||
		(!m_highLimitAllowed && highLimit)
	);
	if (!m_highLimitAllowed)
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
}

bool QUaLimitAlarm::lowLimitAllowed() const
{
	return m_lowLimitAllowed;
}

void QUaLimitAlarm::setLowLimitAllowed(const bool& lowLimitAllowed)
{
	if (lowLimitAllowed == m_lowLimitAllowed)
	{
		return;
	}
	m_lowLimitAllowed = lowLimitAllowed;
	// add or remove component
	auto lowLimit = this->browseChild<QUaProperty>("LowLimit");
	Q_ASSERT(
		(m_lowLimitAllowed && !lowLimit) ||
		(!m_lowLimitAllowed && lowLimit)
	);
	if (!m_lowLimitAllowed)
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
}

bool QUaLimitAlarm::lowLowLimitAllowed() const
{
	return m_lowLowLimitAllowed;
}

void QUaLimitAlarm::setLowLowLimitAllowed(const bool& lowLowLimitAllowed)
{
	if (lowLowLimitAllowed == m_lowLowLimitAllowed)
	{
		return;
	}
	m_lowLowLimitAllowed = lowLowLimitAllowed;
	// add or remove component
	auto lowLowLimit = this->browseChild<QUaProperty>("LowLowLimit");
	Q_ASSERT(
		(m_lowLowLimitAllowed && !lowLowLimit) ||
		(!m_lowLowLimitAllowed && lowLowLimit)
	);
	if (!m_lowLowLimitAllowed)
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
}

bool QUaLimitAlarm::baseHighHighLimitAllowed() const
{
	return m_baseHighHighLimitAllowed;
}

void QUaLimitAlarm::setBaseHighHighLimitAllowed(const bool& baseHighHighLimitAllowed)
{
	if (baseHighHighLimitAllowed == m_baseHighHighLimitAllowed)
	{
		return;
	}
	m_baseHighHighLimitAllowed = baseHighHighLimitAllowed;
	// add or remove component
	auto baseHighHighLimit = this->browseChild<QUaProperty>("BaseHighHighLimit");
	Q_ASSERT(
		(m_baseHighHighLimitAllowed && !baseHighHighLimit) ||
		(!m_baseHighHighLimitAllowed && baseHighHighLimit)
	);
	if (!m_baseHighHighLimitAllowed)
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
}

bool QUaLimitAlarm::baseHighLimitAllowed() const
{
	return m_baseHighLimitAllowed;
}

void QUaLimitAlarm::setBaseHighLimitAllowed(const bool& baseHighLimitAllowed)
{
	if (baseHighLimitAllowed == m_baseHighLimitAllowed)
	{
		return;
	}
	m_baseHighLimitAllowed = baseHighLimitAllowed;
	// add or remove component
	auto baseHighLimit = this->browseChild<QUaProperty>("BaseHighLimit");
	Q_ASSERT(
		(m_baseHighLimitAllowed && !baseHighLimit) ||
		(!m_baseHighLimitAllowed && baseHighLimit)
	);
	if (!m_baseHighLimitAllowed)
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
}

bool QUaLimitAlarm::baseLowLimitAllowed() const
{
	return m_baseLowLimitAllowed;
}

void QUaLimitAlarm::setBaseLowLimitAllowed(const bool& baseLowLimitAllowed)
{
	if (baseLowLimitAllowed == m_baseLowLimitAllowed)
	{
		return;
	}
	m_baseLowLimitAllowed = baseLowLimitAllowed;
	// add or remove component
	auto baseLowLimit = this->browseChild<QUaProperty>("BaseLowLimit");
	Q_ASSERT(
		(m_baseLowLimitAllowed && !baseLowLimit) ||
		(!m_baseLowLimitAllowed && baseLowLimit)
	);
	if (!m_baseLowLimitAllowed)
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
}

bool QUaLimitAlarm::baseLowLowLimitAllowed() const
{
	return m_baseLowLowLimitAllowed;
}

void QUaLimitAlarm::setBaseLowLowLimitAllowed(const bool& baseLowLowLimitAllowed)
{
	if (baseLowLowLimitAllowed == m_baseLowLowLimitAllowed)
	{
		return;
	}
	m_baseLowLowLimitAllowed = baseLowLowLimitAllowed;
	// add or remove component
	auto baseLowLowLimit = this->browseChild<QUaProperty>("BaseLowLowLimit");
	Q_ASSERT(
		(m_baseLowLowLimitAllowed && !baseLowLowLimit) ||
		(!m_baseLowLowLimitAllowed && baseLowLowLimit)
	);
	if (!m_baseLowLowLimitAllowed)
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
}

QUaProperty* QUaLimitAlarm::getHighHighLimit()
{
	return this->browseChild<QUaProperty>("HighHighLimit", true);
}

QUaProperty* QUaLimitAlarm::getHighLimit()
{
	return this->browseChild<QUaProperty>("HighLimit", true);
}

QUaProperty* QUaLimitAlarm::getLowLimit()
{
	return this->browseChild<QUaProperty>("LowLimit", true);
}

QUaProperty* QUaLimitAlarm::getLowLowLimit()
{
	return this->browseChild<QUaProperty>("LowLowLimit", true);
}

QUaProperty* QUaLimitAlarm::getBaseHighHighLimit()
{
	return this->browseChild<QUaProperty>("BaseHighHighLimit", true);
}

QUaProperty* QUaLimitAlarm::getBaseHighLimit()
{
	return this->browseChild<QUaProperty>("BaseHighLimit", true);
}

QUaProperty* QUaLimitAlarm::getBaseLowLimit()
{
	return this->browseChild<QUaProperty>("BaseLowLimit", true);
}

QUaProperty* QUaLimitAlarm::getBaseLowLowLimit()
{
	return this->browseChild<QUaProperty>("BaseLowLowLimit", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS


