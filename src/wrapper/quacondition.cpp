#include "quacondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>
#include <QUaTwoStateVariable>
#include <QUaConditionVariable>

QUaCondition::QUaCondition(
	QUaServer *server
) : QUaBaseEvent(server)
{
	// set default : BaseConditionClassType node id
	this->setConditionClassId(
		QUaTypesConverter::nodeIdToQString(
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASECONDITIONCLASSTYPE)
		)
	);
	// set default : display name of BaseConditionClassType
	this->setConditionClassName("BaseConditionClass");
	// set default : ConditionType browse name
	this->setConditionName(this->typeDefinitionBrowseName());
	// set default : retain false
	this->setRetain(false);
	// set default : disabled state
	this->setEnabledStateFalseState("Disabled");
	this->setEnabledStateTrueState("Enabled");
	this->setEnabledStateCurrentStateName("Disabled");
	this->setEnabledStateId(false);
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// set default : good
	this->setQuality(QUaStatus::Good);
	// set default : 0
	this->setLastSeverity(0);
}

QString QUaCondition::conditionClassId() const
{
	return const_cast<QUaCondition*>(this)->getConditionClassId()->value().toString();
}

void QUaCondition::setConditionClassId(const QString& conditionClassId)
{
	this->getConditionClassId()->setValue(
		conditionClassId,
		QUaStatus::Good,
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

QString QUaCondition::conditionClassName() const
{
	return const_cast<QUaCondition*>(this)->getConditionClassName()->value().toString();
}

void QUaCondition::setConditionClassName(const QString& conditionClassName)
{
	this->getConditionClassName()->setValue(conditionClassName);
}

QString QUaCondition::conditionSubClassId() const
{
	return const_cast<QUaCondition*>(this)->getConditionSubClassId()->value().toString();
}

void QUaCondition::setConditionSubClassId(const QString& conditionSubClassId)
{
	this->getConditionSubClassId()->setValue(
		conditionSubClassId,
		QUaStatusCode(),
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

QString QUaCondition::conditionSubClassName() const
{
	return const_cast<QUaCondition*>(this)->getConditionSubClassName()->value().toString();
}

void QUaCondition::setConditionSubClassName(const QString& conditionSubClassName)
{
	this->getConditionSubClassName()->setValue(conditionSubClassName);
}

QString QUaCondition::conditionName() const
{
	return const_cast<QUaCondition*>(this)->getConditionName()->value().toString();
}

void QUaCondition::setConditionName(const QString& conditionName)
{
	this->getConditionName()->setValue(conditionName);
}

QString QUaCondition::branchId() const
{
	return const_cast<QUaCondition*>(this)->getBranchId()->value().toString();
}

void QUaCondition::setBranchId(const QString& branchId)
{
	this->getBranchId()->setValue(
		branchId,
		QUaStatusCode(),
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

bool QUaCondition::retain() const
{
	return const_cast<QUaCondition*>(this)->getRetain()->value().toBool();
}

void QUaCondition::setRetain(const bool& retain)
{
	this->getRetain()->setValue(retain);
}

QString QUaCondition::enabledStateCurrentStateName() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->currentStateName();
}

void QUaCondition::setEnabledStateCurrentStateName(const QString& enabledState)
{
	this->getEnabledState()->setCurrentStateName(enabledState);
}

bool QUaCondition::enabledStateId() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->id();
}

void QUaCondition::setEnabledStateId(const bool& enabledStateId)
{
	this->getEnabledState()->setId(enabledStateId);
}

QDateTime QUaCondition::enabledStateTransitionTime() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->transitionTime();
}

void QUaCondition::setEnabledStateTransitionTime(const QDateTime& transitionTime)
{
	this->getEnabledState()->setTransitionTime(transitionTime);
}

QString QUaCondition::enabledStateTrueState() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->trueState();
}

void QUaCondition::setEnabledStateTrueState(const QString& trueState)
{
	this->getEnabledState()->setTrueState(trueState);
}

QString QUaCondition::enabledStateFalseState() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->falseState();
}

void QUaCondition::setEnabledStateFalseState(const QString& falseState)
{
	this->getEnabledState()->setFalseState(falseState);
}

QUaStatus QUaCondition::quality() const
{
	return QUaStatus(const_cast<QUaCondition*>(this)->getQuality()->value().toUInt());
}

void QUaCondition::setQuality(const QUaStatus& quality)
{
	this->getQuality()->setValue(
		static_cast<quint32>(quality),
		QUaStatus::Good,
		QDateTime(),
		QDateTime(),
		METATYPE_STATUSCODE
	);
}

quint16 QUaCondition::lastSeverity() const
{
	return const_cast<QUaCondition*>(this)->getLastSeverity()->value().toUInt();
}

void QUaCondition::setLastSeverity(const quint16& lastSeverity)
{
	this->getLastSeverity()->setValue(lastSeverity);
}

void QUaCondition::setSeverity(const quint16& intSeverity)
{
	// set last severity before writing new
	this->setLastSeverity(this->severity());
	// set new severity
	QUaBaseEvent::setSeverity(intSeverity);
}

QString QUaCondition::comment() const
{
	return const_cast<QUaCondition*>(this)->getComment()->value().toString();
}

void QUaCondition::setComment(const QString& comment)
{
	this->getComment()->setValue(comment);
	auto session = this->currentSession();
	if (!session)
	{
		return;
	}
	this->setClientUserId(session->userName());
}

QString QUaCondition::clientUserId() const
{
	return const_cast<QUaCondition*>(this)->getClientUserId()->value().toString();
}

void QUaCondition::setClientUserId(const QString& clientUserId)
{
	this->getClientUserId()->setValue(clientUserId);
}

void QUaCondition::Enable()
{
	// check already enabled
	if (this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONALREADYENABLED);
		return;
	}
	// change EnabledState to Enabled
	this->setEnabledStateCurrentStateName("Enabled");
	this->setEnabledStateId(true);
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// trigger event
	this->setSeverity(0);
	this->setMessage(
		tr("Condition %1 enabled")
		.arg(
			!this->displayName().isEmpty() ? this->displayName() :
			!this->browseName().isEmpty() ? this->browseName() :
			this->nodeId()
		)
	);
	this->trigger();
}

void QUaCondition::Disable()
{
	// check already disabled
	if (!this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONALREADYDISABLED);
		return;
	}
	// change EnabledState to Disabled
	this->setEnabledStateCurrentStateName("Disabled");
	this->setEnabledStateId(false);
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// trigger event
	this->setSeverity(0);
	this->setMessage(
		tr("Condition %1 disabled")
		.arg(
			!this->displayName().isEmpty() ? this->displayName() :
			!this->browseName().isEmpty() ? this->browseName() :
			this->nodeId()
		)
	);
	this->trigger();
}

void QUaCondition::AddComment(QByteArray EventId, QString Comment)
{
	// check given event id matches last event id
	if (EventId != this->eventId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADEVENTIDUNKNOWN);
		return;
	}
	// set last comment
	this->setComment(Comment);
	// set message
	this->setMessage(tr("A comment was added"));
}

QUaProperty* QUaCondition::getConditionClassId()
{
	return this->browseChild<QUaProperty>("ConditionClassId");
}

QUaProperty* QUaCondition::getConditionClassName()
{
	return this->browseChild<QUaProperty>("ConditionClassName");
}

QUaProperty* QUaCondition::getConditionSubClassId()
{
	return this->browseChild<QUaProperty>("ConditionSubClassId", true);
}

QUaProperty* QUaCondition::getConditionSubClassName()
{
	return this->browseChild<QUaProperty>("ConditionSubClassName", true);
}

QUaProperty* QUaCondition::getConditionName()
{
	return this->browseChild<QUaProperty>("ConditionName");
}

QUaProperty* QUaCondition::getBranchId()
{
	return this->browseChild<QUaProperty>("BranchId");
}

QUaProperty* QUaCondition::getRetain()
{
	return this->browseChild<QUaProperty>("Retain");
}

QUaTwoStateVariable* QUaCondition::getEnabledState()
{
	return this->browseChild<QUaTwoStateVariable>("EnabledState");
}

QUaConditionVariable* QUaCondition::getQuality()
{
	return this->browseChild<QUaConditionVariable>("Quality");
}

QUaConditionVariable* QUaCondition::getLastSeverity()
{
	return this->browseChild<QUaConditionVariable>("LastSeverity");
}

QUaConditionVariable* QUaCondition::getComment()
{
	return this->browseChild<QUaConditionVariable>("Comment");
}

QUaProperty* QUaCondition::getClientUserId()
{
	return this->browseChild<QUaProperty>("ClientUserId");
}




#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS