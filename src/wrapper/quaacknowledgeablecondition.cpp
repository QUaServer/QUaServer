#include "quaacknowledgeablecondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaTwoStateVariable>
#include <QUaConditionVariable>

QUaAcknowledgeableCondition::QUaAcknowledgeableCondition(
	QUaServer *server
) : QUaCondition(server)
{
	// ConfirmedState default set when instantiating because is optional
	m_confirmRequired = false;
	// resue rest of defaults 
	this->resetInternals();
}

QUaLocalizedText QUaAcknowledgeableCondition::ackedStateCurrentStateName() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->currentStateName();
}

void QUaAcknowledgeableCondition::setAckedStateCurrentStateName(const QUaLocalizedText& ackedState)
{
	this->getAckedState()->setCurrentStateName(ackedState);
}

bool QUaAcknowledgeableCondition::ackedStateId() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->id();
}

void QUaAcknowledgeableCondition::setAckedStateId(const bool& ackedStateId)
{
	this->getAckedState()->setId(ackedStateId);
}

QDateTime QUaAcknowledgeableCondition::ackedStateTransitionTime() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->transitionTime();
}

void QUaAcknowledgeableCondition::setAckedStateTransitionTime(const QDateTime& transitionTime)
{
	this->getAckedState()->setTransitionTime(transitionTime);
}

QUaLocalizedText QUaAcknowledgeableCondition::ackedStateTrueState() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->trueState();
}

void QUaAcknowledgeableCondition::setAckedStateTrueState(const QUaLocalizedText& trueState)
{
	this->getAckedState()->setTrueState(trueState);
}

QUaLocalizedText QUaAcknowledgeableCondition::ackedStateFalseState() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->falseState();
}

void QUaAcknowledgeableCondition::setAckedStateFalseState(const QUaLocalizedText& falseState)
{
	this->getAckedState()->setFalseState(falseState);
}

bool QUaAcknowledgeableCondition::acknowledged() const
{
	return this->ackedStateId();
}

void QUaAcknowledgeableCondition::setAcknowledged(const bool& acknowledged)
{
	// change AckedState to Acknowledged
	auto strAckedStateName = acknowledged ?
		this->ackedStateTrueState() :
		this->ackedStateFalseState();
	this->setAckedStateCurrentStateName(strAckedStateName);
	this->setAckedStateId(acknowledged);
	this->setAckedStateTransitionTime(this->getAckedState()->serverTimestamp());
	// if no further attention needed, set retain to false if branch then delete as well
	QString strMessage = tr("Condition %1.").arg(strAckedStateName);
	if (acknowledged && !this->requiresAttention())
	{
		if (this->isBranch())
		{
			this->deleteLater();
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
	}
	if (m_confirmRequired && !this->confirmed())
	{
		strMessage += tr(" Requires Confirm.");
	}
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(strMessage);
	this->trigger();
	// emit qt signal
	emit this->conditionAcknowledged();
}

QUaLocalizedText QUaAcknowledgeableCondition::confirmedStateCurrentStateName() const
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return QUaLocalizedText();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->currentStateName();
}

void QUaAcknowledgeableCondition::setConfirmedStateCurrentStateName(const QUaLocalizedText& confirmedState)
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setCurrentStateName(confirmedState);
}

bool QUaAcknowledgeableCondition::confirmedStateId() const
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return false;
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->id();
}

void QUaAcknowledgeableCondition::setConfirmedStateId(const bool& confirmedStateId)
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setId(confirmedStateId);
}

QDateTime QUaAcknowledgeableCondition::confirmedStateTransitionTime() const
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return QDateTime();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->transitionTime();
}

void QUaAcknowledgeableCondition::setConfirmedStateTransitionTime(const QDateTime& transitionTime)
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setTransitionTime(transitionTime);
}

QUaLocalizedText QUaAcknowledgeableCondition::confirmedStateTrueState() const
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return QUaLocalizedText();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->trueState();
}

void QUaAcknowledgeableCondition::setConfirmedStateTrueState(const QUaLocalizedText& trueState)
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setTrueState(trueState);
}

QUaLocalizedText QUaAcknowledgeableCondition::confirmedStateFalseState() const
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return QUaLocalizedText();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->falseState();
}

void QUaAcknowledgeableCondition::setConfirmedStateFalseState(const QUaLocalizedText& falseState)
{
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setFalseState(falseState);
}

bool QUaAcknowledgeableCondition::confirmed() const
{
	return this->confirmedStateId();
}

void QUaAcknowledgeableCondition::setConfirmed(const bool& confirmed)
{
	// change ConfirmedState to Confirmed
	auto strConfirmedStateName = confirmed ?
		this->confirmedStateTrueState() :
		this->confirmedStateFalseState();
	this->setConfirmedStateCurrentStateName(strConfirmedStateName);
	this->setConfirmedStateId(confirmed);
	this->setConfirmedStateTransitionTime(this->getConfirmedState()->serverTimestamp());
	// if no further attention needed, set retain to false if branch then delete as well
	QString strMessage = tr("Condition %1.").arg(strConfirmedStateName);
	if (confirmed && !this->requiresAttention())
	{
		if (this->isBranch())
		{
			this->deleteLater();
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
	}
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(strMessage);
	this->trigger();
	// emit qt signal
	emit this->conditionConfirmed();
}

void QUaAcknowledgeableCondition::Acknowledge(QByteArray EventId, QUaLocalizedText Comment)
{
	// check if enabled
	if (!this->enabled())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONDISABLED);
		return;
	}
	// check given EventId matches, if ampty assume method call on this instance
	if (!EventId.isEmpty() && EventId != this->eventId())
	{
		auto branch = this->branchByEventId<QUaAcknowledgeableCondition>(EventId);
		if (branch)
		{
			branch->Acknowledge(EventId, Comment);
			return;
		}
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADEVENTIDUNKNOWN);
		return;
	}
	// check already acked
	if (this->ackedStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONBRANCHALREADYACKED);
		return;
	}
	// set comment (no trigger)
	this->getComment()->QUaBaseVariable::setValue(Comment);
	// update user id if applicable
	auto session = this->currentSession();
	if (session)
	{
		this->setClientUserId(session->userName());
	}
	// change AckedState to Acked and trigger
	this->setAcknowledged(true);
}

void QUaAcknowledgeableCondition::Confirm(QByteArray EventId, QUaLocalizedText Comment)
{
	// check if need to block progammatic called
	if (!m_confirmRequired)
	{
		// TODO : log error message
		return;
	}
	// check if enabled
	if (!this->enabled())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONDISABLED);
		return;
	}
	// check given EventId matches, if ampty assume method call on this instance
	if (!EventId.isEmpty() && EventId != this->eventId())
	{
		auto branch = this->branchByEventId<QUaAcknowledgeableCondition>(EventId);
		if (branch)
		{
			branch->Confirm(EventId, Comment);
			return;
		}
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADEVENTIDUNKNOWN);
		return;
	}
	// check already confirmed
	if (this->confirmedStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONBRANCHALREADYCONFIRMED);
		return;
	}
	// check already acked
	if (!this->ackedStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADSTATENOTACTIVE);
		return;
	}
	// set comment (no trigger)
	this->getComment()->QUaBaseVariable::setValue(Comment);
	// update user id if applicable
	auto session = this->currentSession();
	if (session)
	{
		this->setClientUserId(session->userName());
	}
	// change ConfirmedState to Confirmed and trigger
	this->setConfirmed(true);
}

void QUaAcknowledgeableCondition::resetInternals()
{
	QUaCondition::resetInternals();
	// set default : Unacknowledged state
	this->setAckedStateFalseState(tr("Unacknowledged"));
	this->setAckedStateTrueState(tr("Acknowledged"));
	this->setAckedStateCurrentStateName(this->ackedStateTrueState());
	this->setAckedStateId(true);
	this->setAckedStateTransitionTime(this->getAckedState()->serverTimestamp());
	// check if confirm allowed
	if (!m_confirmRequired)
	{
		return;
	}
	// initialize and set defaults
	this->setConfirmedStateFalseState(tr("Unconfirmed"));
	this->setConfirmedStateTrueState(tr("Confirmed"));
	this->setConfirmedStateCurrentStateName(this->confirmedStateTrueState());
	this->setConfirmedStateId(true);
	this->setConfirmedStateTransitionTime(this->getConfirmedState()->serverTimestamp());
}

bool QUaAcknowledgeableCondition::confirmRequired() const
{
	Q_ASSERT(m_confirmRequired == this->hasOptionalMethod("Confirm"));
	return m_confirmRequired;
}
bool QUaAcknowledgeableCondition::requiresAttention() const
{
	// base implementation
	bool requiresAttention = QUaCondition::requiresAttention();
	// check acknowledged
	requiresAttention = requiresAttention || !this->ackedStateId();
	// check confirmed
	if (m_confirmRequired)
	{
		requiresAttention = requiresAttention || !this->confirmedStateId();
	}
	// return result
	return requiresAttention;
}

void QUaAcknowledgeableCondition::setConfirmRequired(const bool& confirmRequired)
{
	if (confirmRequired == m_confirmRequired)
	{
		return;
	}
	m_confirmRequired = confirmRequired;
	// add or remove method
	Q_ASSERT(
		(m_confirmRequired  && !this->hasOptionalMethod("Confirm")) ||
		(!m_confirmRequired && this->hasOptionalMethod("Confirm"))
	);
	m_confirmRequired ?
		this->addOptionalMethod("Confirm") :
		this->removeOptionalMethod("Confirm");
	// add or remove ConfirmedState component
	auto confirmedState = this->browseChild<QUaTwoStateVariable>("ConfirmedState");
	Q_ASSERT(
		(m_confirmRequired && !confirmedState) ||
		(!m_confirmRequired && confirmedState)
	);
	if (!m_confirmRequired)
	{
		Q_CHECK_PTR(confirmedState);
		// remove
		delete confirmedState;
		return;
	}
	// initialize and set defaults
	this->setConfirmedStateFalseState(tr("Unconfirmed"));
	this->setConfirmedStateTrueState(tr("Confirmed"));
	this->setConfirmedStateCurrentStateName(this->confirmedStateTrueState());
	this->setConfirmedStateId(true);
	this->setConfirmedStateTransitionTime(this->getConfirmedState()->serverTimestamp());
}

QUaTwoStateVariable* QUaAcknowledgeableCondition::getAckedState()
{
	return this->browseChild<QUaTwoStateVariable>("AckedState");
}

QUaTwoStateVariable* QUaAcknowledgeableCondition::getConfirmedState()
{
	return this->browseChild<QUaTwoStateVariable>("ConfirmedState", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS