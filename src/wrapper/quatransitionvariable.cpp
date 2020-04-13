#include "quatransitionvariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaTransitionVariable::QUaTransitionVariable(
	QUaServer* server
) : QUaBaseDataVariable(server)
{
	
}

QVariant QUaTransitionVariable::id() const
{
	return const_cast<QUaTransitionVariable*>(this)->getId()->value();
}

void QUaTransitionVariable::setId(const QVariant& id)
{
	this->getId()->setValue(id);
}

QUaQualifiedName QUaTransitionVariable::name() const
{
	return const_cast<QUaTransitionVariable*>(this)->getName()->value<QUaQualifiedName>();
}

void QUaTransitionVariable::setName(const QUaQualifiedName& name)
{
	this->getName()->setValue(QVariant::fromValue(name));
}

quint32 QUaTransitionVariable::number() const
{
	return const_cast<QUaTransitionVariable*>(this)->getNumber()->value().toUInt();
}

void QUaTransitionVariable::setNumber(const quint32& number)
{
	this->getNumber()->setValue(number);
}

QDateTime QUaTransitionVariable::transitionTime() const
{
	return const_cast<QUaTransitionVariable*>(this)->getTransitionTime()->value().toDateTime();
}

void QUaTransitionVariable::setTransitionTime(const QDateTime& transitionTime)
{
	this->getTransitionTime()->setValue(transitionTime);
}

QDateTime QUaTransitionVariable::effectiveTransitionTime() const
{
	return const_cast<QUaTransitionVariable*>(this)->getEffectiveTransitionTime()->value().toDateTime();
}

void QUaTransitionVariable::setEffectiveTransitionTime(const QDateTime& effectiveTransitionTime)
{
	this->getEffectiveTransitionTime()->setValue(effectiveTransitionTime);
}

QUaProperty* QUaTransitionVariable::getId()
{
	return this->browseChild<QUaProperty>("Id");
}

QUaProperty* QUaTransitionVariable::getName()
{
	return this->browseChild<QUaProperty>("Name", true);
}

QUaProperty* QUaTransitionVariable::getNumber()
{
	return this->browseChild<QUaProperty>("Number", true);
}

QUaProperty* QUaTransitionVariable::getTransitionTime()
{
	return this->browseChild<QUaProperty>("TransitionTime", true);
}

QUaProperty* QUaTransitionVariable::getEffectiveTransitionTime()
{
	return this->browseChild<QUaProperty>("EffectiveTransitionTime", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS