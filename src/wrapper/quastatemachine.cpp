#include "quastatemachine.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaStateVariable>
#include <QUaTransitionVariable>

QUaStateMachine::QUaStateMachine(
	QUaServer* server
) : QUaBaseObject(server)
{
	
}

QUaLocalizedText QUaStateMachine::currentState() const
{
	return const_cast<QUaStateMachine*>(this)->getCurrentState()->value<QUaLocalizedText>();
}

void QUaStateMachine::setCurrentState(const QUaLocalizedText& currentState)
{
	this->getCurrentState()->setValue(currentState);
}

QUaLocalizedText QUaStateMachine::lastTransition() const
{
	return const_cast<QUaStateMachine*>(this)->getLastTransition()->value<QUaLocalizedText>();
}

void QUaStateMachine::setLastTransition(const QUaLocalizedText& lastTransition)
{
	this->getLastTransition()->setValue(lastTransition);
}

QUaStateVariable* QUaStateMachine::getCurrentState()
{
	return this->browseChild<QUaStateVariable>("CurrentState");
}

QUaTransitionVariable* QUaStateMachine::getLastTransition()
{
	return this->browseChild<QUaTransitionVariable>("LastTransition", true);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS