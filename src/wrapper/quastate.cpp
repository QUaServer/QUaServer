#include "quastate.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaState::QUaState(
	QUaServer* server
) : QUaBaseObject(server)
{
	
}

quint32 QUaState::stateNumber() const
{
	return const_cast<QUaState*>(this)->getStateNumber()->value<quint32>();
}

void QUaState::setStateNumber(const quint32& stateNumber)
{
	this->getStateNumber()->setValue(stateNumber);
}

QUaProperty* QUaState::getStateNumber()
{
	return this->browseChild<QUaProperty>("StateNumber");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS