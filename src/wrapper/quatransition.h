#ifndef QUATRANSITION_H
#define QUATRANSITION_H

#include <QUaBaseObject>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaTransition : public QUaBaseObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaTransition(
		QUaServer* server
	);

	quint32 transitionNumber() const;
	void    setTransitionNumber(const quint32& transitionNumber);

protected:
	QUaProperty* getTransitionNumber();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUATRANSITION_H

