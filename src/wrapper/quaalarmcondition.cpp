#include "quaalarmcondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaTwoStateVariable>
#include <QUaConditionVariable>

QUaAlarmCondition::QUaAlarmCondition(
	QUaServer *server
) : QUaAcknowledgeableCondition(server)
{
	m_inputNode = nullptr;
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

void QUaAlarmCondition::setActive(const bool& active, const QString& strMessageAppend/* = QString()*/)
{
	// nothing to do if same
	if (active == this->active())
	{
		return;
	}
	// activate or deactivate
	auto strActiveStateName = active ?
		this->activeStateTrueState() :
		this->activeStateFalseState();
	auto time = QDateTime::currentDateTimeUtc();
	this->setActiveStateId(active);
	this->setActiveStateCurrentStateName(strActiveStateName);
	this->setActiveStateTransitionTime(time);
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
		this->setMessage(
			strMessageAppend.isEmpty() ?
			tr("Alarm %1. Requires Acknowledge.").arg(strActiveStateName) :
			tr("Alarm %1. %2 Requires Acknowledge.").arg(strActiveStateName).arg(strMessageAppend)
		);
	}
	else
	{
		QString strMessage = strMessageAppend.isEmpty() ? 
			tr("Alarm %1.").arg(strActiveStateName) :
			tr("Alarm %1. %2").arg(strActiveStateName).arg(strMessageAppend);
		if (this->requiresAttention())
		{
			// create branch
			auto branch = this->createBranch<QUaAlarmConditionBranch>();
			this->setRetain(true);
			if (!this->acknowledged())
			{
				strMessage += tr(" Requires Acknowledge.");
			}
			else if (m_confirmRequired && !this->confirmed())
			{
				strMessage += tr(" Requires Confirm.");
			}
			if (branch)
			{
				strMessage += tr(" Has branches.");
			}
		}
		else
		{
			bool hasBranches = this->hasBranches();
			this->setRetain(hasBranches);
			if (hasBranches)
			{
				strMessage += tr(" Has branches.");
			}
		}
		this->setMessage(strMessage);
	}
	// trigger event
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
/***************************************************************************************
*/

QUaBrowsePath QUaAlarmConditionBranch::ActiveState               ({ { 0, "ActiveState" } }); // [LocalizedText]
QUaBrowsePath QUaAlarmConditionBranch::ActiveState_Id            ({ { 0, "ActiveState" },{ 0, "Id"             } }); // [Boolean]
//QUaBrowsePath QUaAlarmConditionBranch::ActiveState_FalseState    ({ { 0, "ActiveState" },{ 0, "FalseState"     } }); // [LocalizedText]
//QUaBrowsePath QUaAlarmConditionBranch::ActiveState_TrueState     ({ { 0, "ActiveState" },{ 0, "TrueState"      } }); // [LocalizedText]
QUaBrowsePath QUaAlarmConditionBranch::ActiveState_TransitionTime({ { 0, "ActiveState" },{ 0, "TransitionTime" } }); // [UtcTime]

QUaAlarmConditionBranch::QUaAlarmConditionBranch(
	QUaCondition* parent,
	const QUaNodeId& branchId
) : QUaAcknowledgeableConditionBranch(parent, branchId)
{
	
}

bool QUaAlarmConditionBranch::active() const
{
	return this->value(QUaAlarmConditionBranch::ActiveState_Id).value<bool>();
}

bool QUaAlarmConditionBranch::requiresAttention() const
{
	// base implementation
	bool requiresAttention = QUaAcknowledgeableConditionBranch::requiresAttention();
	// if active
	requiresAttention = requiresAttention || this->active();
	// return result
	return requiresAttention;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS