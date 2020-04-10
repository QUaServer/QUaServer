#include "quaexclusivelimitstatemachine.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaExclusiveLimitStateMachine::QUaExclusiveLimitStateMachine(
	QUaServer* server
) : QUaFiniteStateMachine(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS