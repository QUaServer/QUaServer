#include "quaalarmcondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaTwoStateVariable>
#include <QUaConditionVariable>

QUaAlarmCondition::QUaAlarmCondition(
	QUaServer *server
) : QUaAcknowledgeableCondition(server)
{
	// resue rest of defaults 
	this->resetInternals();
}

QUaAlarmCondition::~QUaAlarmCondition()
{
	this->cleanConnections();
}

QUaLocalizedText QUaAlarmCondition::activeStateCurrentStateName() const
{
	return const_cast<QUaAlarmCondition*>(this)->getActiveState()->currentStateName();
}

void QUaAlarmCondition::setActiveStateCurrentStateName(const QUaLocalizedText& activeState)
{
	this->getActiveState()->setCurrentStateName(activeState);
}

bool QUaAlarmCondition::activeStateId() const
{
	return const_cast<QUaAlarmCondition*>(this)->getActiveState()->id();
}

void QUaAlarmCondition::setActiveStateId(const bool& activeStateId)
{
	this->getActiveState()->setId(activeStateId);
}

QDateTime QUaAlarmCondition::activeStateTransitionTime() const
{
	return const_cast<QUaAlarmCondition*>(this)->getActiveState()->transitionTime();
}

void QUaAlarmCondition::setActiveStateTransitionTime(const QDateTime& transitionTime)
{
	this->getActiveState()->setTransitionTime(transitionTime);
}

QUaLocalizedText QUaAlarmCondition::activeStateTrueState() const
{
	return const_cast<QUaAlarmCondition*>(this)->getActiveState()->trueState();
}

void QUaAlarmCondition::setActiveStateTrueState(const QUaLocalizedText& trueState)
{
	this->getActiveState()->setTrueState(trueState);
}

QUaLocalizedText QUaAlarmCondition::activeStateFalseState() const
{
	return const_cast<QUaAlarmCondition*>(this)->getActiveState()->falseState();
}

void QUaAlarmCondition::setActiveStateFalseState(const QUaLocalizedText& falseState)
{
	this->getActiveState()->setFalseState(falseState);
}

QUaNodeId QUaAlarmCondition::inputNode() const
{
	return const_cast<QUaAlarmCondition*>(this)->getInputNode()->value<QUaNodeId>();
}

void QUaAlarmCondition::setInputNode(const QUaNodeId& inputNodeId)
{
	QUaNode * node = m_qUaServer->nodeById(inputNodeId);
	QUaBaseVariable* var = qobject_cast<QUaBaseVariable*>(node);
	if (node && !var)
	{
		Q_ASSERT_X(false, "QUaAlarmCondition::setInputNode", "Input node must be a variable.");
		// TODO : error log
	}
	this->setInputNode(var);
}

void QUaAlarmCondition::setInputNode(QUaBaseVariable* inputNode)
{
	this->cleanConnections();
	m_inputNode = inputNode;
	// set nodeId
	this->getInputNode()->setValue(m_inputNode ? m_inputNode->nodeId() : QUaNodeId());
	// subscribe to source changes
	if (!m_inputNode)
	{
		return;
	}
	m_connections <<
	QObject::connect(m_inputNode, &QObject::destroyed, this,
	[this]() {
		this->cleanConnections();
		this->setInputNode(nullptr);
	});
}

bool QUaAlarmCondition::suppressedOrShelve() const
{
	return const_cast<QUaAlarmCondition*>(this)->getSuppressedOrShelve()->value<bool>();
}

void QUaAlarmCondition::setSuppressedOrShelve(const bool& suppressedOrShelve)
{
	this->getSuppressedOrShelve()->setValue(suppressedOrShelve);
}

void QUaAlarmCondition::Silence()
{
	// TODO 
}

void QUaAlarmCondition::Suppress()
{
	// TODO 
}

void QUaAlarmCondition::Unsuppress()
{
	// TODO 
}

void QUaAlarmCondition::RemoveFromService()
{
	// TODO 
}

void QUaAlarmCondition::PlaceInService()
{
	// TODO 
}

void QUaAlarmCondition::Reset()
{
	// TODO 
}

bool QUaAlarmCondition::active() const
{
	return this->activeStateId();
}

void QUaAlarmCondition::setActive(const bool& active)
{
	// nothing to do if same
	if (active == this->active())
	{
		return;
	}
	// activate or deactivate
	this->setActiveStateId(active);
	auto strActiveStateName = active ?
		this->activeStateTrueState() :
		this->activeStateFalseState();
	this->setActiveStateCurrentStateName(strActiveStateName);
	this->setActiveStateTransitionTime(this->getActiveState()->serverTimestamp());	
	// special logic for main branch
	if (!this->isBranch())
	{
		// if active have to retain and reset acknowledged and confirmed
		if (active)
		{
			this->setRetain(true);
			// reset aknowledged
			this->setAckedStateCurrentStateName(this->ackedStateFalseState());
			this->setAckedStateId(false);
			this->setAckedStateTransitionTime(this->getAckedState()->serverTimestamp());
			// reset confirmed
			if (this->confirmRequired())
			{
				this->setConfirmedStateCurrentStateName(this->confirmedStateFalseState());
				this->setConfirmedStateId(false);
				this->setConfirmedStateTransitionTime(this->getConfirmedState()->serverTimestamp());
			}
			this->setMessage(tr("Alarm %1. Requires Acknowledge.").arg(strActiveStateName));
		}
		else
		{
			if (this->requiresAttention())
			{
				// create branch
				this->createBranch<QUaAlarmCondition>();
				this->setRetain(true);
			}
			else
			{
				this->setRetain(this->hasBranches());
			}
			this->setMessage(tr("Alarm %1. Has branches.").arg(strActiveStateName));
		}
	}
	else
	{
		this->setMessage(tr("Alarm %1.").arg(strActiveStateName));
	}
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	// NOTE : message set according to situation
	this->trigger();
	// emit qt signal
	active ?
		emit this->activated() :
		emit this->deactivated();
}

void QUaAlarmCondition::cleanConnections()
{
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
}

QUaTwoStateVariable* QUaAlarmCondition::getActiveState()
{
	return this->browseChild<QUaTwoStateVariable>("ActiveState");
}

QUaProperty* QUaAlarmCondition::getInputNode()
{
	return this->browseChild<QUaProperty>("InputNode");
}

QUaProperty* QUaAlarmCondition::getSuppressedOrShelve()
{
	return this->browseChild<QUaProperty>("SuppressedOrShelve");
}

bool QUaAlarmCondition::requiresAttention() const
{
	// base implementation
	bool requiresAttention = QUaAcknowledgeableCondition::requiresAttention();
	// if active
	requiresAttention = requiresAttention || this->active();
	// return result
	return requiresAttention;
}

void QUaAlarmCondition::resetInternals()
{
	QUaAcknowledgeableCondition::resetInternals();
	// set default : Inactive state
	this->setActiveStateFalseState(tr("Inactive"));
	this->setActiveStateTrueState(tr("Active"));
	this->setActiveStateCurrentStateName(tr("Inactive"));
	this->setActiveStateId(false);
	this->setActiveStateTransitionTime(this->getActiveState()->serverTimestamp());
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS