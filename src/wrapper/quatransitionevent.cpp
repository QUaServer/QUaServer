#include "quatransitionevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaTransitionVariable>
#include <QUaStateVariable>

QUaTransitionEvent::QUaTransitionEvent(
	QUaServer *server
) : QUaBaseEvent(server)
{
	
}

QUaLocalizedText QUaTransitionEvent::transition() const
{
	return const_cast<QUaTransitionEvent*>(this)->getTransition()->value<QUaLocalizedText>();
}

void QUaTransitionEvent::setTransition(const QUaLocalizedText& transition)
{
	this->getTransition()->setValue(transition);
}

QUaLocalizedText QUaTransitionEvent::fromState() const
{
	return const_cast<QUaTransitionEvent*>(this)->getFromState()->value<QUaLocalizedText>();
}

void QUaTransitionEvent::setFromState(const QUaLocalizedText& fromState)
{
	this->getFromState()->setValue(fromState);
}

QUaLocalizedText QUaTransitionEvent::toState() const
{
	return const_cast<QUaTransitionEvent*>(this)->getToState()->value<QUaLocalizedText>();
}

void QUaTransitionEvent::setToState(const QUaLocalizedText& toState)
{
	this->getToState()->setValue(toState);
}

QUaTransitionVariable* QUaTransitionEvent::getTransition()
{
	return this->browseChild<QUaTransitionVariable>("Transition");
}

QUaStateVariable* QUaTransitionEvent::getFromState()
{
	return this->browseChild<QUaStateVariable>("FromState");
}

QUaStateVariable* QUaTransitionEvent::getToState()
{
	return this->browseChild<QUaStateVariable>("ToState");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
