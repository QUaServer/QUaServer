#include "quaoffnormalalarm.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaOffNormalAlarm::QUaOffNormalAlarm(
	QUaServer* server
) : QUaDiscreteAlarm(server)
{
	
}

void QUaOffNormalAlarm::setInputNode(QUaBaseVariable* inputNode)
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
		if (!m_normalValue.isValid())
		{
			return;
		}
		this->setActive(value != m_normalValue);
	});
}

QVariant QUaOffNormalAlarm::normalValue() const
{
	return m_normalValue;
}

void QUaOffNormalAlarm::setNormalValue(const QVariant& normalValue)
{
	if (normalValue == this->normalValue())
	{
		return;
	}
	m_normalValue = normalValue;
	// trigger active state recalculation
	if (!m_inputNode)
	{
		return;
	}
	emit m_inputNode->valueChanged(m_inputNode->value(), false);
}

QUaNodeId QUaOffNormalAlarm::normalState() const
{
	return const_cast<QUaOffNormalAlarm*>(this)->getNormalState()->value<QUaNodeId>();
}

void QUaOffNormalAlarm::setNormalState(const QUaNodeId& normalState)
{
	this->getNormalState()->setValue(normalState);
}

QUaProperty* QUaOffNormalAlarm::getNormalState()
{
	return this->browseChild<QUaProperty>("NormalState");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS