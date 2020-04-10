#ifndef QUATRANSITION_H
#define QUATRANSITION_H

#include <QUaBaseObject>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.10 
/*


*/

class QUaTransition : public QUaBaseObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaTransition(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUATRANSITION_H

