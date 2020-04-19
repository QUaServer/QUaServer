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
	// check if enabled
	if (!this->enabledStateId())
	{
		return;
	}
	// create branch (BEFORE deactivate)
	QUaAlarmCondition* branch = nullptr;
	if (!active && !this->isBranch() && this->requiresAttention())
	{
		branch = this->createBranch<QUaAlarmCondition>();
	}
	// activate or deactivate
	this->setActiveStateId(active);
	QString strActiveStateName = active ?
		this->activeStateTrueState() :
		this->activeStateFalseState();
	this->setActiveStateCurrentStateName(strActiveStateName);
	this->setActiveStateTransitionTime(this->getActiveState()->serverTimestamp());
	// check if trigger
	if (!this->shouldTrigger())
	{
		return;
	}
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setMessage(tr("Alarm %1").arg(strActiveStateName));
	this->setTime(time);
	this->setReceiveTime(time);
	this->trigger();
	// emit qt signal
	active ?
		emit this->activated() :
		emit this->deactivated();
	// trigger another event if a branch was created
	if (!branch)
	{
		return;
	}
	branch->setMessage(tr("Previous state requires attention, branch %1 created").arg(branch->branchId()));
	branch->trigger();
	// TODO : try to reproduce example B.1.3
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
	// TODO : are there any conditions?
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