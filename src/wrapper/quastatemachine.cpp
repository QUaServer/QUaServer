#include "quastatemachine.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaStateMachine::QUaStateMachine(
	QUaServer* server
) : QUaBaseObject(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS