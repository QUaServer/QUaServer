#include "quatransitionvariable.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaTransitionVariable::QUaTransitionVariable(
	QUaServer* server
) : QUaBaseDataVariable(server)
{
	
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS