#include "quaexclusivelimitalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaExclusiveLimitStateMachine>

QUaExclusiveLimitAlarm::QUaExclusiveLimitAlarm(
	QUaServer* server
) : QUaLimitAlarm(server)
{
	auto machine = const_cast<QUaExclusiveLimitAlarm*>(this)->getLimitState();
	// forward signal
	QObject::connect(
		machine, &QUaExclusiveLimitStateMachine::exclusiveLimitStateChanged,
		this, &QUaExclusiveLimitAlarm::exclusiveLimitStateChanged
	);
}

void QUaExclusiveLimitAlarm::setInputNode(QUaBaseVariable* inputNode)
{
	// call base implementation
	QUaAlarmCondition::setInputNode(inputNode);
	if (!inputNode)
	{
		return;
	}
	// subscribe to value changes
	m_connections <<
	QObject::connect(m_inputNode, &QUaBaseVariable::valueChanged, this,
	[this](const QVariant& value) {
		Q_ASSERT(value.canConvert<double>());
		this->processInputNodeValue(value.value<double>());
	});
}

QUaExclusiveLimitState QUaExclusiveLimitAlarm::exclusiveLimitState() const
{
	auto machine = const_cast<QUaExclusiveLimitAlarm*>(this)->getLimitState();
	return machine->exclusiveLimitState();
}

void QUaExclusiveLimitAlarm::setExclusiveLimitState(const QUaExclusiveLimitState& exclusiveLimitState)
{
	auto machine = this->getLimitState();
	auto oldState = machine->exclusiveLimitState();
	if (exclusiveLimitState == oldState)
	{
		return;
	}
	if (!this->isExclusiveLimitStateAllowed(exclusiveLimitState))
	{
		Q_ASSERT(false);
		// TODO : error log
		return;
	}
	machine->setExclusiveLimitState(exclusiveLimitState);
	// trigger enabled/disabled
	if (oldState == QUa::ExclusiveLimitState::None &&
		exclusiveLimitState != QUa::ExclusiveLimitState::None)
	{
		this->setActive(true);
	}
	else if (oldState != QUa::ExclusiveLimitState::None &&
		exclusiveLimitState == QUa::ExclusiveLimitState::None)
	{
		this->setActive(false);
	}
}

bool QUaExclusiveLimitAlarm::isExclusiveLimitStateAllowed(const QUaExclusiveLimitState& exclusiveLimitState)
{
	if (exclusiveLimitState == QUa::ExclusiveLimitState::HighHigh)
	{
		return this->highHighLimitAllowed();
	}
	if (exclusiveLimitState == QUa::ExclusiveLimitState::High)
	{
		return this->highLimitAllowed();
	}
	if (exclusiveLimitState == QUa::ExclusiveLimitState::Low)
	{
		return this->lowLimitAllowed();
	}
	if (exclusiveLimitState == QUa::ExclusiveLimitState::LowLow)
	{
		return this->lowLowLimitAllowed();
	}
	return true;
}

void QUaExclusiveLimitAlarm::processInputNodeValue(const double& value)
{
	bool highHighLimitAllowed = this->highHighLimitAllowed();
	bool highLimitAllowed     = this->highLimitAllowed    ();
	bool lowLimitAllowed      = this->lowLimitAllowed     ();
	bool lowLowLimitAllowed   = this->lowLowLimitAllowed  ();
	QUaExclusiveLimitState newState;
	if (highLimitAllowed && value >= this->highLimit())
	{
		newState = QUa::ExclusiveLimitState::High;
	}
	if (highHighLimitAllowed && value >= this->highHighLimit())
	{
		newState = QUa::ExclusiveLimitState::HighHigh;
	}
	if (lowLimitAllowed && value <= this->lowLimit())
	{
		newState = QUa::ExclusiveLimitState::Low;
	}
	if (lowLowLimitAllowed && value <= this->lowLowLimit())
	{
		newState = QUa::ExclusiveLimitState::LowLow;
	}
	this->setExclusiveLimitState(newState);
}

void QUaExclusiveLimitAlarm::setHighHighLimitAllowed(const bool& highHighLimitAllowed)
{
	// call base class implementation
	QUaLimitAlarm::setHighHighLimitAllowed(highHighLimitAllowed);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setHighHighLimitAllowed(highHighLimitAllowed);
}


void QUaExclusiveLimitAlarm::setHighLimitAllowed(const bool& highLimitAllowed)
{
	// call base class implementation
	QUaLimitAlarm::setHighLimitAllowed(highLimitAllowed);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setHighLimitAllowed(highLimitAllowed);
}

void QUaExclusiveLimitAlarm::setLowLimitAllowed(const bool& lowLimitAllowed)
{
	// call base class implementation
	QUaLimitAlarm::setLowLimitAllowed(lowLimitAllowed);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setLowLimitAllowed(lowLimitAllowed);
}

void QUaExclusiveLimitAlarm::setLowLowLimitAllowed(const bool& lowLowLimitAllowed)
{
	// call base class implementation
	QUaLimitAlarm::setLowLowLimitAllowed(lowLowLimitAllowed);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setLowLowLimitAllowed(lowLowLimitAllowed);
}


QUaExclusiveLimitStateMachine* QUaExclusiveLimitAlarm::getLimitState()
{
	return this->browseChild<QUaExclusiveLimitStateMachine>("LimitState");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS