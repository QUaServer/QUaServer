#include "quaconditionvariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaProperty>

const QStringList QUaConditionVariable::mandatoryChildrenBrowseNames()
{
	return QUaBaseDataVariable::mandatoryChildrenBrowseNames() + QStringList()
		<< "SourceTimestamp";
}

QUaConditionVariable::QUaConditionVariable(
	QUaServer* server,
	const MC& mandatoryChildren
) : QUaBaseDataVariable(server, mandatoryChildren)
{
	// set child property initial value
	this->getSourceTimestamp()->setValue(QUaBaseVariable::sourceTimestamp());
	// update child property if value changed through netowrk
	QObject::connect(
		this, &QUaBaseVariable::sourceTimestampChanged, 
		this, &QUaConditionVariable::on_setSourceTimestampChanged
	);
}

void QUaConditionVariable::setValue(
	const QVariant& value, 
	const QDateTime& sourceTimestamp, 
	const QDateTime& serverTimestamp, 
	const QMetaType::Type& newType)
{
	// call base implementation
	QUaBaseVariable::setValue(value, sourceTimestamp, serverTimestamp, newType);
	// update child property
	this->getSourceTimestamp()->setValue(QUaBaseVariable::sourceTimestamp());
}

void QUaConditionVariable::setSourceTimestamp(const QDateTime& sourceTimestamp)
{
	// call base implementation
	QUaBaseVariable::setSourceTimestamp(sourceTimestamp);
	// update child property
	this->getSourceTimestamp()->setValue(sourceTimestamp.toUTC());
}

void QUaConditionVariable::on_setSourceTimestampChanged(const QDateTime& sourceTimestamp)
{
	// update child property
	this->getSourceTimestamp()->setValue(sourceTimestamp.toUTC());
}

QUaProperty* QUaConditionVariable::getSourceTimestamp()
{
	return this->findChild<QUaProperty*>("SourceTimestamp");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS