#include "quafinitestatemachine.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteStateMachine::QUaFiniteStateMachine(
	QUaServer* server
) : QUaStateMachine(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS