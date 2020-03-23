#include "quaconditionvariable.h"

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

}

QDateTime QUaConditionVariable::sourceTimestamp() const
{
	return const_cast<QUaConditionVariable*>(this)->getSourceTimestamp()->value().toDateTime().toUTC();
}

void QUaConditionVariable::setSourceTimestamp(const QDateTime& dateTime)
{
	this->getSourceTimestamp()->setValue(dateTime.toUTC());
}

QUaProperty* QUaConditionVariable::getSourceTimestamp()
{
	return this->findChild<QUaProperty*>("SourceTimestamp");
}
