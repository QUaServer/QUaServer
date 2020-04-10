#include "quafinitetransitionvariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteTransitionVariable::QUaFiniteTransitionVariable(
	QUaServer* server
) : QUaTransitionVariable(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS