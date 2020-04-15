#ifndef QUASTATE_H
#define QUASTATE_H

#include <QUaBaseObject>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaProperty;

class QUaState : public QUaBaseObject
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit QUaState(
		QUaServer* server
	);

	quint32 stateNumber() const;
	void    setStateNumber(const quint32& stateNumber);

protected:
	QUaProperty* getStateNumber();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUASTATE_H

