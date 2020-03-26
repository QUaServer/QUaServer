#include "quatwostatevariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaTwoStateVariable::QUaTwoStateVariable(
	QUaServer* server
) : QUaStateVariable(server)
{
	
}

QString QUaTwoStateVariable::currentStateName() const
{
	return this->value().toString();
}

void QUaTwoStateVariable::setCurrentStateName(const QString& currentStateName)
{
	this->setValue(
		currentStateName,
		QDateTime(),
		QDateTime(),
		METATYPE_LOCALIZEDTEXT
	);
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

QString QUaTwoStateVariable::trueState() const
{
	return const_cast<QUaTwoStateVariable*>(this)->getTrueState()->value().toString();
}

void QUaTwoStateVariable::setTrueState(const QString& trueState)
{
	this->getTrueState()->setValue(trueState);
}

QString QUaTwoStateVariable::falseState() const
{
	return const_cast<QUaTwoStateVariable*>(this)->getFalseState()->value().toString();
}

void QUaTwoStateVariable::setFalseState(const QString& falseState)
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