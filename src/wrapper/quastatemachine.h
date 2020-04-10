#ifndef QUASTATEMACHINE_H
#define QUASTATEMACHINE_H

#include <QUaBaseObject>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

// Part 5 - B.4.2
/*


*/

class QUaStateMachine : public QUaBaseObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaStateMachine(
		QUaServer* server
	);


protected:


};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUASTATEMACHINE_H

