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
	// support optional last transtion
	machine->lastTransition();
	// subscribe to limit changes to force alarm recalculation
	QObject::connect(this, &QUaLimitAlarm::highHighLimitRequiredChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
	QObject::connect(this, &QUaLimitAlarm::highHighLimitChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
	QObject::connect(this, &QUaLimitAlarm::highLimitRequiredChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
	QObject::connect(this, &QUaLimitAlarm::highLimitChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
	QObject::connect(this, &QUaLimitAlarm::lowLimitRequiredChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
	QObject::connect(this, &QUaLimitAlarm::lowLimitChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
	QObject::connect(this, &QUaLimitAlarm::lowLowLimitRequiredChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
	QObject::connect(this, &QUaLimitAlarm::lowLowLimitChanged, this,
		[this]() {
			this->forceActiveStateRecalculation();
		});
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
		this->setActive(
			true, 
			tr("%1 limit violation.").arg(exclusiveLimitState.toString())
		);
	}
	else if (oldState != QUa::ExclusiveLimitState::None &&
		exclusiveLimitState == QUa::ExclusiveLimitState::None)
	{
		this->setActive(
			false, 
			this->requiresAttention() ? 
				tr("%1 limit violation.").arg(oldState.toString()) :
				QString()
		);
	}
	else
	{
		Q_ASSERT(this->active());
		QString strMessage = tr("Alarm %1.").arg(this->activeStateTrueState());
		strMessage += tr(" %2 limit violation.").arg(exclusiveLimitState.toString());
		if (!this->acknowledged())
		{
			strMessage += tr(" Requires Acknowledge.");
		}
		else if (m_confirmRequired && !this->confirmed())
		{
			strMessage += tr(" Requires Confirm.");
		}
		this->setMessage(strMessage);
		// trigger event
		auto time = QDateTime::currentDateTimeUtc();
		this->setTime(time);
		this->setReceiveTime(time);
		// NOTE : message set according to situation
		this->trigger();
	}
}

bool QUaExclusiveLimitAlarm::isExclusiveLimitStateAllowed(const QUaExclusiveLimitState& exclusiveLimitState)
{
	if (exclusiveLimitState == QUa::ExclusiveLimitState::HighHigh)
	{
		return this->highHighLimitRequired();
	}
	if (exclusiveLimitState == QUa::ExclusiveLimitState::High)
	{
		return this->highLimitRequired();
	}
	if (exclusiveLimitState == QUa::ExclusiveLimitState::Low)
	{
		return this->lowLimitRequired();
	}
	if (exclusiveLimitState == QUa::ExclusiveLimitState::LowLow)
	{
		return this->lowLowLimitRequired();
	}
	return true;
}

void QUaExclusiveLimitAlarm::processInputNodeValue(const double& value)
{
	bool highHighLimitRequired = this->highHighLimitRequired();
	bool highLimitRequired     = this->highLimitRequired    ();
	bool lowLimitRequired      = this->lowLimitRequired     ();
	bool lowLowLimitRequired   = this->lowLowLimitRequired  ();
	QUaExclusiveLimitState newState;
	if (highLimitRequired && value >= this->highLimit())
	{
		newState = QUa::ExclusiveLimitState::High;
	}
	if (highHighLimitRequired && value >= this->highHighLimit())
	{
		newState = QUa::ExclusiveLimitState::HighHigh;
	}
	if (lowLimitRequired && value <= this->lowLimit())
	{
		newState = QUa::ExclusiveLimitState::Low;
	}
	if (lowLowLimitRequired && value <= this->lowLowLimit())
	{
		newState = QUa::ExclusiveLimitState::LowLow;
	}
	this->setExclusiveLimitState(newState);
}

void QUaExclusiveLimitAlarm::forceActiveStateRecalculation()
{
	if (!m_inputNode)
	{
		return;
	}
	emit m_inputNode->valueChanged(m_inputNode->value(), false);
}

void QUaExclusiveLimitAlarm::setHighHighLimitRequired(const bool& highHighLimitRequired)
{
	// call base class implementation
	QUaLimitAlarm::setHighHighLimitRequired(highHighLimitRequired);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setHighHighLimitRequired(highHighLimitRequired);
}


void QUaExclusiveLimitAlarm::setHighLimitRequired(const bool& highLimitRequired)
{
	// call base class implementation
	QUaLimitAlarm::setHighLimitRequired(highLimitRequired);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setHighLimitRequired(highLimitRequired);
}

void QUaExclusiveLimitAlarm::setLowLimitRequired(const bool& lowLimitRequired)
{
	// call base class implementation
	QUaLimitAlarm::setLowLimitRequired(lowLimitRequired);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setLowLimitRequired(lowLimitRequired);
}

void QUaExclusiveLimitAlarm::setLowLowLimitRequired(const bool& lowLowLimitRequired)
{
	// call base class implementation
	QUaLimitAlarm::setLowLowLimitRequired(lowLowLimitRequired);
	// update available states and transations in state machine
	auto machine = this->getLimitState();
	machine->setLowLowLimitRequired(lowLowLimitRequired);
}

QUaExclusiveLimitStateMachine* QUaExclusiveLimitAlarm::getLimitState()
{
	return this->browseChild<QUaExclusiveLimitStateMachine>("LimitState");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS