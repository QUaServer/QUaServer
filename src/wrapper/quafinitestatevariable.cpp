#include "quafinitestatevariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaFiniteStateVariable::QUaFiniteStateVariable(
	QUaServer* server
) : QUaStateVariable(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS