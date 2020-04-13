#include "quatwostatevariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaTwoStateVariable::QUaTwoStateVariable(
	QUaServer* server
) : QUaStateVariable(server)
{
	
}

QUaLocalizedText QUaTwoStateVariable::currentStateName() const
{
	return this->value().value<QUaLocalizedText>();
}

void QUaTwoStateVariable::setCurrentStateName(const QUaLocalizedText& currentStateName)
{
	this->setValue(currentStateName);
}

bool QUaTwoStateVariable::id()
{
	return QUaStateVariable::id().toBool();
}

void QUaTwoStateVariable::setId(const bool& id)
{
	QUaStateVariable::setId(id);
}

QDateTime QUaTwoStateVariable::transitionTime() const
{
	return const_cast<QUaTwoStateVariable*>(this)->getTransitionTime()->value().toDateTime().toUTC();
}

void QUaTwoStateVariable::setTransitionTime(const QDateTime& transitionTime)
{
	this->getTransitionTime()->setValue(transitionTime.toUTC());
}

QDateTime QUaTwoStateVariable::effectiveTransitionTime() const
{
	return const_cast<QUaTwoStateVariable*>(this)->getEffectiveTransitionTime()->value().toDateTime().toUTC();
}

void QUaTwoStateVariable::setEffectiveTransitionTime(const QDateTime& effectiveTransitionTime)
{
	this->getEffectiveTransitionTime()->setValue(effectiveTransitionTime.toUTC());
}

QUaLocalizedText QUaTwoStateVariable::trueState() const
{
	return const_cast<QUaTwoStateVariable*>(this)->getTrueState()->value<QUaLocalizedText>();
}

void QUaTwoStateVariable::setTrueState(const QUaLocalizedText& trueState)
{
	this->getTrueState()->setValue(trueState);
}

QUaLocalizedText QUaTwoStateVariable::falseState() const
{
	return const_cast<QUaTwoStateVariable*>(this)->getFalseState()->value<QUaLocalizedText>();
}

void QUaTwoStateVariable::setFalseState(const QUaLocalizedText& falseState)
{
	this->getFalseState()->setValue(falseState);
}

QUaProperty* QUaTwoStateVariable::getTransitionTime()
{
	return this->browseChild<QUaProperty>("TransitionTime", true);
}

QUaProperty* QUaTwoStateVariable::getEffectiveTransitionTime()
{
	return this->browseChild<QUaProperty>("EffectiveTransitionTime", true);
}

QUaProperty* QUaTwoStateVariable::getTrueState()
{
	return this->browseChild<QUaProperty>("TrueState", true);
}

QUaProperty* QUaTwoStateVariable::getFalseState()
{
	return this->browseChild<QUaProperty>("FalseState", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS