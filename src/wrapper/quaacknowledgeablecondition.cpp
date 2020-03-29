#include "quaacknowledgeablecondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>
#include <QUaTwoStateVariable>
#include <QUaConditionVariable>

QUaAcknowledgeableCondition::QUaAcknowledgeableCondition(
	QUaServer *server
) : QUaCondition(server)
{
	// ConfirmedState default set when instantiating because is optional
	m_confirmAllowed = false;
	// resue rest of defaults 
	this->reset();
}

QString QUaAcknowledgeableCondition::ackedStateCurrentStateName() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->currentStateName();
}

void QUaAcknowledgeableCondition::setAckedStateCurrentStateName(const QString& ackedState)
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

QString QUaAcknowledgeableCondition::ackedStateTrueState() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->trueState();
}

void QUaAcknowledgeableCondition::setAckedStateTrueState(const QString& trueState)
{
	this->getAckedState()->setTrueState(trueState);
}

QString QUaAcknowledgeableCondition::ackedStateFalseState() const
{
	return const_cast<QUaAcknowledgeableCondition*>(this)->getAckedState()->falseState();
}

void QUaAcknowledgeableCondition::setAckedStateFalseState(const QString& falseState)
{
	this->getAckedState()->setFalseState(falseState);
}


QString QUaAcknowledgeableCondition::confirmedStateCurrentStateName() const
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return QString();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->currentStateName();
}

void QUaAcknowledgeableCondition::setConfirmedStateCurrentStateName(const QString& confirmedState)
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setCurrentStateName(confirmedState);
}

bool QUaAcknowledgeableCondition::confirmedStateId() const
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return false;
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->id();
}

void QUaAcknowledgeableCondition::setConfirmedStateId(const bool& confirmedStateId)
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setId(confirmedStateId);
}

QDateTime QUaAcknowledgeableCondition::confirmedStateTransitionTime() const
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return QDateTime();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->transitionTime();
}

void QUaAcknowledgeableCondition::setConfirmedStateTransitionTime(const QDateTime& transitionTime)
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setTransitionTime(transitionTime);
}

QString QUaAcknowledgeableCondition::confirmedStateTrueState() const
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return QString();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->trueState();
}

void QUaAcknowledgeableCondition::setConfirmedStateTrueState(const QString& trueState)
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setTrueState(trueState);
}

QString QUaAcknowledgeableCondition::confirmedStateFalseState() const
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return QString();
	}
	return const_cast<QUaAcknowledgeableCondition*>(this)->getConfirmedState()->falseState();
}

void QUaAcknowledgeableCondition::setConfirmedStateFalseState(const QString& falseState)
{
	if (!m_confirmAllowed)
	{
		// TODO : log error message
		return;
	}
	this->getConfirmedState()->setFalseState(falseState);
}

void QUaAcknowledgeableCondition::Acknowledge(QByteArray EventId, QString Comment)
{
	// check if enabled
	if (!this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONDISABLED);
		return;
	}
	// check already acked
	if (this->ackedStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONBRANCHALREADYACKED);
		return;
	}
	// change AckedState to Acknowledged
	this->setAckedStateCurrentStateName("Acknowledged");
	this->setAckedStateId(true);
	this->setAckedStateTransitionTime(this->getAckedState()->serverTimestamp());
	// set comment
	this->setComment(Comment);
	// trigger event
	this->setSeverity(100);
	this->setMessage(tr("Condition acknowledged"));
	this->setTime(QDateTime::currentDateTime().toUTC());
	this->trigger();
	// emit qt signal
	emit this->acknowledged();
}

void QUaAcknowledgeableCondition::Confirm(QByteArray EventId, QString Comment)
{
	// check if enabled
	if (!this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONDISABLED);
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

	// change ConfirmedState to Acknowledged
	this->setConfirmedStateCurrentStateName("Confirmed");
	this->setConfirmedStateId(true);
	this->setConfirmedStateTransitionTime(this->getConfirmedState()->serverTimestamp());
	// set comment
	this->setComment(Comment);
	// trigger event
	this->setSeverity(100);
	this->setMessage(tr("Condition confirmed"));
	this->setTime(QDateTime::currentDateTime().toUTC());
	this->trigger();
	// emit qt signal
	emit this->confirmed();
}

void QUaAcknowledgeableCondition::reset()
{
	QUaCondition::reset();
	// set default : Unacknowledged state
	this->setAckedStateFalseState("Unacknowledged");
	this->setAckedStateTrueState("Acknowledged");
	this->setAckedStateCurrentStateName("Unacknowledged");
	this->setAckedStateId(false);
	this->setAckedStateTransitionTime(this->getAckedState()->serverTimestamp());
	// check if confirm allowed
	if (!m_confirmAllowed)
	{
		return;
	}
	// initialize and set defaults
	this->setConfirmedStateFalseState("Unconfirmed");
	this->setConfirmedStateTrueState("Confirmed");
	this->setConfirmedStateCurrentStateName("Unconfirmed");
	this->setConfirmedStateId(false);
	this->setConfirmedStateTransitionTime(this->getAckedState()->serverTimestamp());
}

bool QUaAcknowledgeableCondition::confirmAllowed() const
{
	Q_ASSERT(m_confirmAllowed == this->hasOptionalMethod("Confirm"));
	return m_confirmAllowed;
}

void QUaAcknowledgeableCondition::setConfirmAllowed(const bool& confirmAllowed)
{
	if (confirmAllowed == m_confirmAllowed)
	{
		return;
	}
	m_confirmAllowed = confirmAllowed;
	// add or remove method
	Q_ASSERT(
		(m_confirmAllowed  && !this->hasOptionalMethod("Confirm")) ||
		(!m_confirmAllowed && this->hasOptionalMethod("Confirm"))
	);
	m_confirmAllowed ?
		this->addOptionalMethod("Confirm") :
		this->removeOptionalMethod("Confirm");
	// add or remove ConfirmedState component
	auto confirmedState = this->browseChild<QUaTwoStateVariable>("ConfirmedState");
	Q_ASSERT(
		(m_confirmAllowed && !confirmedState) ||
		(!m_confirmAllowed && confirmedState)
	);
	if (!m_confirmAllowed)
	{
		Q_CHECK_PTR(confirmedState);
		// remove
		delete confirmedState;
		return;
	}
	// initialize and set defaults
	this->setConfirmedStateFalseState("Unconfirmed");
	this->setConfirmedStateTrueState("Confirmed");
	this->setConfirmedStateCurrentStateName("Unconfirmed");
	this->setConfirmedStateId(false);
	this->setConfirmedStateTransitionTime(this->getAckedState()->serverTimestamp());
}

QUaTwoStateVariable* QUaAcknowledgeableCondition::getAckedState()
{
	return this->browseChild<QUaTwoStateVariable>("AckedState");
}

QUaTwoStateVariable* QUaAcknowledgeableCondition::getConfirmedState()
{
	return this->browseChild<QUaTwoStateVariable>("ConfirmedState", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS