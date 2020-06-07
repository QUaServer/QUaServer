#ifndef QUAALARMCONDITION_H
#define QUAALARMCONDITION_H

#include <QUaAcknowledgeableCondition>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaBaseVariable;

class QUaAlarmCondition : public QUaAcknowledgeableCondition
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaAlarmCondition(
		QUaServer *server
	);
	~QUaAlarmCondition();

	// children

	// If condition currently exists
	QUaLocalizedText activeStateCurrentStateName() const;
	void             setActiveStateCurrentStateName(const QUaLocalizedText& activeState);
	bool             activeStateId() const;
	void             setActiveStateId(const bool& activeStateId);
	QDateTime        activeStateTransitionTime() const;
	void             setActiveStateTransitionTime(const QDateTime& transitionTime);
	QUaLocalizedText activeStateTrueState() const;
	void             setActiveStateTrueState(const QUaLocalizedText& trueState);
	QUaLocalizedText activeStateFalseState() const;
	void             setActiveStateFalseState(const QUaLocalizedText& falseState);
	// helper
	bool active() const;
	void setActive(const bool& active, const QString& strMessageAppend = QString());

	// NodeId of the Variable which is used for the calculation of the Alarm state.
	// Can be null if internal
	QUaNodeId inputNode() const;
	void      setInputNode(const QUaNodeId& QUaNodeId);
	// Helper
	virtual void setInputNode(QUaBaseVariable* inputNode);

	// TODO : SuppressedState
	// TODO : OutOfServiceState
	// TODO : ShelvingState

	bool suppressedOrShelve() const;
	void setSuppressedOrShelve(const bool& suppressedOrShelve);

	// TODO : MaxTimeShelved
	// TODO : AudibleEnabled
	// TODO : AudibleSound
	// TODO : SilenceState
	// TODO : OnDelay
	// TODO : OffDelay
	// TODO : FirstInGroupFlag
	// TODO : FirstInGroup
	// TODO : LatchedState
	// TODO : <AlarmGroup>
	// TODO : ReAlarmTime
	// TODO : ReAlarmRepeatCount

	// methods

	Q_INVOKABLE void Silence();
	
	Q_INVOKABLE void Suppress();
	
	Q_INVOKABLE void Unsuppress();
	
	Q_INVOKABLE void RemoveFromService();
	
	Q_INVOKABLE void PlaceInService();
	
	Q_INVOKABLE void Reset();


signals:
	void activated();
	void deactivated();

protected:
	QUaBaseVariable* m_inputNode;
	QList<QMetaObject::Connection> m_connections;
	void cleanConnections();
	// LocalizedText
	QUaTwoStateVariable* getActiveState();
	// NodeId
	QUaProperty* getInputNode();

	// TODO : LocalizedText, getSuppressedState
	// TODO : LocalizedText, getOutOfServiceState
	// TODO : ShelvedStateMachineType, getShelvingState

	// Boolean
	QUaProperty* getSuppressedOrShelve();

	// TODO : Duration, getMaxTimeShelved

	// TODO : Boolean, getAudibleEnabled
	// TODO : AudioDataType, AudibleSound

	// TODO : LocalizedText, getSilenceState

	// TODO : Duration, getOnDelay
	// TODO : Duration, getOffDelay

	// TODO : Boolean, getFirstInGroupFlag
	// TODO : AlarmGroupType, getFirstInGroup
	// TODO : LocalizedText, getLatchedState

	// TODO : AlarmGroupType, get<AlarmGroup>

	// TODO : Duration, getReAlarmTime
	// TODO : Int16, getReAlarmRepeatCount

	// helpers

	// reimplement to define branch delete conditions
	virtual bool requiresAttention() const;
	// reimplement to reset type internals (QUaAlarmCondition::Reset)
	virtual void resetInternals();
};

class QUaAlarmConditionBranch : public QUaAcknowledgeableConditionBranch
{
public:
	QUaAlarmConditionBranch(
		QUaCondition* parent,
		const QUaNodeId& branchId = QUaNodeId()
	);

	bool active() const;

protected:

	// reimplement
	virtual bool requiresAttention() const override;

	// QUaAlarmCondition
	static QUaBrowsePath ActiveState;
	static QUaBrowsePath ActiveState_Id;
	//static QUaBrowsePath ActiveState_FalseState;
	//static QUaBrowsePath ActiveState_TrueState;
	static QUaBrowsePath ActiveState_TransitionTime;

	friend QUaAlarmCondition;
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAALARMCONDITION_H
