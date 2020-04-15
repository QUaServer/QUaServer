#ifndef QUAACKNOWLEDGEABLECONDITION_H
#define QUAACKNOWLEDGEABLECONDITION_H

#include <QUaCondition>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaAcknowledgeableCondition : public QUaCondition
{
    Q_OBJECT

	Q_PROPERTY(bool confirmAllowed READ confirmAllowed WRITE setConfirmAllowed)

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaAcknowledgeableCondition(
		QUaServer *server
	);

	// children

	QString   ackedStateCurrentStateName() const;
	void      setAckedStateCurrentStateName(const QString& ackedState);
	bool      ackedStateId() const;
	void      setAckedStateId(const bool& ackedStateId);
	QDateTime ackedStateTransitionTime() const;
	void      setAckedStateTransitionTime(const QDateTime& transitionTime);
	QString   ackedStateTrueState() const;
	void      setAckedStateTrueState(const QString& trueState);
	QString   ackedStateFalseState() const;
	void      setAckedStateFalseState(const QString& falseState);
	// helper sets setAckedStateId, setAckedStateCurrentStateName, setAckedStateTransitionTime 
	// and triggers event according to specification
	// NOTE : change of the Acked state must be normally make by the client through
	//        the use of the Acknowledge() and QUaAlarmCondition::Reset() methods
	bool      acknowledged() const;
	void      setAcknowledged(const bool& acknowledged);

	// NOTE: optional, only work if confirmAllowed == true 
	QString   confirmedStateCurrentStateName() const;
	void      setConfirmedStateCurrentStateName(const QString& confirmedState);
	bool      confirmedStateId() const;
	void      setConfirmedStateId(const bool& confirmedStateId);
	QDateTime confirmedStateTransitionTime() const;
	void      setConfirmedStateTransitionTime(const QDateTime& transitionTime);
	QString   confirmedStateTrueState() const;
	void      setConfirmedStateTrueState(const QString& trueState);
	QString   confirmedStateFalseState() const;
	void      setConfirmedStateFalseState(const QString& falseState);
	// helper sets setConfirmedStateId, setConfirmedStateCurrentStateName, setConfirmedStateTransitionTime 
	// and triggers event according to specification
	// NOTE : change of the Confirmed state must be normally make by the client through
	//        the use of the Confirm() and QUaAlarmCondition::Reset() methods
	bool      confirmed() const;
	void      setConfirmed(const bool& confirmed);

	// methods

	Q_INVOKABLE void Acknowledge(QByteArray EventId, QString Comment);

	Q_INVOKABLE void Confirm(QByteArray EventId, QString Comment);

	// helpers

	bool confirmAllowed() const;
	void setConfirmAllowed(const bool & confirmAllowed);

signals:
	void acknowledged();

	void confirmed();

protected:
	bool m_confirmAllowed;
	// LocalizedText
	QUaTwoStateVariable* getAckedState();
	// LocalizedText
	QUaTwoStateVariable* getConfirmedState();

	// helpers

	// reimplement to define branch delete conditions
	virtual bool canDeleteBranch() const;
	// reimplement to reset type internals (QUaAlarmCondition::Reset)
	virtual void resetInternals();

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAACKNOWLEDGEABLECONDITION_H