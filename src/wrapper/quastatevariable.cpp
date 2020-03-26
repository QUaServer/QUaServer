#include "quastatevariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

UA_VariableAttributes QUaStateVariable::m_vAttr = []() {
	// register address
	QString strClassName = QString(QUaStateVariable::staticMetaObject.className());
	QUaServer::m_hashDefAttrs[strClassName] = &QUaStateVariable::m_vAttr;
	// init
	auto vAttr = UA_VariableAttributes_default;
	vAttr.valueRank = UA_VALUERANK_SCALAR;
	return vAttr;
}();

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

QString QUaStateVariable::name() const
{
	return const_cast<QUaStateVariable*>(this)->getName()->value().toString();
}

void QUaStateVariable::setName(const QString& name)
{
	this->getName()->setValue(name);
}

quint32 QUaStateVariable::number() const
{
	return const_cast<QUaStateVariable*>(this)->getNumber()->value().toUInt();
}

void QUaStateVariable::setNumber(const quint32& number)
{
	this->getNumber()->setValue(number);
}

QString QUaStateVariable::effectiveDisplayName() const
{
	return const_cast<QUaStateVariable*>(this)->getEffectiveDisplayName()->value().toString();
}

void QUaStateVariable::setEffectiveDisplayName(const QString& effectiveDisplayName)
{
	this->getEffectiveDisplayName()->setValue(
		effectiveDisplayName,
		QDateTime(),
		QDateTime(),
		METATYPE_LOCALIZEDTEXT
	);
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