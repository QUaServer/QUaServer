#ifndef QUAALARMCONDITION_H
#define QUAALARMCONDITION_H

#include <QUaAcknowledgeableCondition>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

/*
Part 9 - 5.8.2 AlarmConditionType (abstract = false)

Extends the AcknowledgeableConditionType by introducing an ActiveState, SuppressedState and ShelvingState. 
It also adds the ability to set a delay time, re-alarm time, Alarm groups and audible Alarm settings.

An Alarm in the Active state indicates that the situation the Condition is representing currently
exists. When an Alarm is an inactive state it is representing a situation that has returned to a
normal state

Subtypes of the AlarmConditionType, will have sub-state models that further define the Active state.
For example, a multi-state level Alarm when in the Active state may be in one of the following 
sub-states: LowLow, Low, High or HighHigh.

HasComponent             | Variable | ActiveState        | LocalizedText    | TwoStateVariableType    | Mandatory
HasProperty              | Variable | InputNode          | NodeId           | PropertyType            | Mandatory
HasComponent             | Variable | SuppressedState    | LocalizedText    | TwoStateVariableType    | Optional
HasComponent             | Variable | OutOfServiceState  | LocalizedText    | TwoStateVariableType    | Optional
														 				  
HasComponent             | Object   | ShelvingState      |                  | ShelvedStateMachineType | Optional
HasProperty              | Variable | SuppressedOrShelve | Boolean          | PropertyType            | Mandatory
HasProperty              | Variable | MaxTimeShelved     | Duration         | PropertyType            | Optional
														 				  
HasProperty              | Variable | AudibleEnabled     | Boolean          | PropertyType            | Optional
HasProperty              | Variable | AudibleSound       | AudioDataType    | AudioVariableType       | Optional
HasComponent             | Variable | SilenceState       | LocalizedText    | TwoStateVariableType    | Optional
HasProperty              | Variable | OnDelay            | Duration         | PropertyType            | Optional
HasProperty              | Variable | OffDelay           | Duration         | PropertyType            | Optional
														 				  
HasComponent             | Variable | FirstInGroupFlag   | Boolean          | BaseDataVariableType    | Optional
HasComponent             | Object   | FirstInGroup       |                  | AlarmGroupType          | Optional
HasComponent             | Object   | LatchedState       | LocalizedText    | TwoStateVariableType    | Optional
HasAlarmSuppressionGroup | Object   | <AlarmGroup>       |                  | AlarmGroupType          | OptionalPlaceholder
														 				  
HasProperty              | Variable | ReAlarmTime        | Duration         | PropertyType            | Optional
HasComponent             | Variable | ReAlarmRepeatCount | Int16            | BaseDataVariableType    | Optional
														 				  
HasComponent             | Method   | Silence            | Defined in 5.8.5                           | Optional
HasComponent             | Method   | Suppress           | Defined in 5.8.6                           | Optional
HasComponent             | Method   | Unsuppress         | Defined in 5.8.7                           | Optional
HasComponent             | Method   | RemoveFromService  | Defined in 5.8.8                           | Optional
HasComponent             | Method   | PlaceInService     | Defined in 5.8.9                           | Optional
HasComponent             | Method   | Reset              | Defined in 5.8.4                           | Optional


The shelving state can be set by an Operator via OPC UA Methods. The suppressed state is
set internally by the Server due to system specific reasons. Alarm systems typically implement
the suppress, out of service and shelve features to help keep Operators from being
overwhelmed during Alarm “storms” by limiting the number of Alarms an Operator sees on a
current Alarm display. This is accomplished by setting the SuppressedOrShelved flag on second
order dependent Alarms and/or Alarms of less severity, leading the Operator to concentrate on
the most critical issues.

The shelved, out of service and suppressed states differ from the Disabled state in that Alarms
are still fully functional and can be included in Subscription Notifications to a Client

*/

class QUaAlarmCondition : public QUaAcknowledgeableCondition
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaAlarmCondition(
		QUaServer *server
	);

	// children

	// ActiveState/Id when set to True indicates that the situation the Condition is representing
	// currently exists. When a Condition instance is in the inactive state(ActiveState / Id when set to
	// False) it is representing a situation that has returned to a normal state.The transitions of
	// Conditions to the inactiveand Active states are triggered by Server specific actions.
	QString   activeStateCurrentStateName() const;
	void      setActiveStateCurrentStateName(const QString& activeState);
	bool      activeStateId() const;
	void      setActiveStateId(const bool& activeStateId);
	QDateTime activeStateTransitionTime() const;
	void      setActiveStateTransitionTime(const QDateTime& transitionTime);
	QString   activeStateTrueState() const;
	void      setActiveStateTrueState(const QString& trueState);
	QString   activeStateFalseState() const;
	void      setActiveStateFalseState(const QString& falseState);

	// The InputNode Property provides the NodeId of the Variable the Value of which is used as
	// primary input in the calculation of the Alarm state.If this Variable is not in the AddressSpace,
	// a NULL NodeId shall be provided.In some systems, an Alarm may be calculated based on
	// multiple Variables Values; it is up to the system to determine which Variable’s NodeId is used
	QString inputNode() const;
	void    setInputNode(const QString& inputNodeId);

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

	// The Silence Method is used silence a specific Alarm instance. It is only available on an instance
	// of an AlarmConditionType that also exposes the SilenceState.
	Q_INVOKABLE void Silence();
	
	// The Suppress Method is used to suppress a specific Alarm instance. It is only available on an
	// instance of an AlarmConditionType that also exposes the SuppressedState.This Method can
	// be used to change the SuppressedState of an Alarmand overwrite any suppression caused by
	// an associated AlarmSuppressionGroup.This Method works in parallel with any suppression
	// triggered by an AlarmSupressionGroup, in that if the Method is used to suppress an Alarm, an
	// AlarmSuppressionGroup might clear the suppression.
	Q_INVOKABLE void Suppress();
	
	// The Unsuppress Method is used to clear the SuppressedState of a specific Alarm instance. It
	// is only available on an instance of an AlarmConditionType that also exposes the
	// SuppressedState.This Method can be used to overwrite any suppression cause by an
	// associated AlarmSuppressionGroup.This Method works in parallel with any suppression
	// triggered by an AlarmSuppressionGroup, in that if the Method is used to clear the
	// SuppressedState of an Alarm, any change in an AlarmSuppressionGroup might again suppress
	// the Alarm.
	Q_INVOKABLE void Unsuppress();
	
	// The RemoveFromService Method is used to suppress a specific Alarm instance. It is only
	// available on an instance of an AlarmConditionType that also exposes the OutOfServiceState.
	Q_INVOKABLE void RemoveFromService();
	
	// The PlaceInService Method is used to set the OutOfServiceState to False of a specific Alarm
	// instance.It is only available on an instance of an AlarmConditionType that also exposes the
	// OutOfServiceState.
	Q_INVOKABLE void PlaceInService();
	
	// The Reset Method is used reset a latched Alarm instance. It is only available on an instance of
	// an AlarmConditionType that exposes the LatchedState.
	Q_INVOKABLE void Reset();
	


	// helpers

	virtual void resetInternals() override;

	bool active() const;
	void setActive(const bool& active);

signals:
	void activated();
	void deactivated();


protected:
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

};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAALARMCONDITION_H