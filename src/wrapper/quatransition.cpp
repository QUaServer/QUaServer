#include "quatransition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaTransition::QUaTransition(
	QUaServer* server
) : QUaBaseObject(server)
{
	
}

quint32 QUaTransition::transitionNumber() const
{
	return const_cast<QUaTransition*>(this)->getTransitionNumber()->value<quint32>();
}

void QUaTransition::setTransitionNumber(const quint32& transitionNumber)
{
	this->getTransitionNumber()->setValue(transitionNumber);
}

QUaProperty* QUaTransition::getTransitionNumber()
{
	return this->browseChild<QUaProperty>("TransitionNumber");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS