#include "quastatevariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaStateVariable::QUaStateVariable(
	QUaServer* server
) : QUaBaseDataVariable(server)
{

}

QVariant QUaStateVariable::id() const
{
	return const_cast<QUaStateVariable*>(this)->getId()->value();
}

void QUaStateVariable::setId(const QVariant& id)
{
	this->getId()->setValue(id);
}

QUaQualifiedName QUaStateVariable::name() const
{
	return const_cast<QUaStateVariable*>(this)->getName()->value<QUaQualifiedName>();
}

void QUaStateVariable::setName(const QUaQualifiedName& name)
{
	this->getName()->setValue(QVariant::fromValue(name));
}

quint32 QUaStateVariable::number() const
{
	return const_cast<QUaStateVariable*>(this)->getNumber()->value<quint32>();
}

void QUaStateVariable::setNumber(const quint32& number)
{
	this->getNumber()->setValue(number);
}

QUaLocalizedText QUaStateVariable::effectiveDisplayName() const
{
	return const_cast<QUaStateVariable*>(this)->getEffectiveDisplayName()->value<QUaLocalizedText>();
}

void QUaStateVariable::setEffectiveDisplayName(const QUaLocalizedText& effectiveDisplayName)
{
	this->getEffectiveDisplayName()->setValue(effectiveDisplayName);
}

QUaProperty* QUaStateVariable::getId()
{
	return this->browseChild<QUaProperty>("Id");
}

QUaProperty* QUaStateVariable::getName()
{
	return this->browseChild<QUaProperty>("Name", true);
}

QUaProperty* QUaStateVariable::getNumber()
{
	return this->browseChild<QUaProperty>("Number", true);
}

QUaProperty* QUaStateVariable::getEffectiveDisplayName()
{
	return this->browseChild<QUaProperty>("EffectiveDisplayName", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS