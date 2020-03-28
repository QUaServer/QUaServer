#ifndef QUACONDITION_H
#define QUACONDITION_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part 9 - 5.5 ConditionType (abstract = true)

The Condition model extends the Event model by defining the ConditionType.
The ConditionType introduces the concept of states differentiating it from the base Event model.
Unlike the BaseEventType, Conditions are not transient. The ConditionType is further extended
into Dialog and AcknowledgeableConditionType, each of which has their own sub-types.

The False state of the EnabledState shall not be extended with a sub state machine.

HasProperty  | Variable | ConditionClassId      | NodeId          | PropertyType          | Mandatory
HasProperty  | Variable | ConditionClassName    | LocalizedText   | PropertyType          | Mandatory
HasProperty  | Variable | ConditionSubClassId   | NodeId[]        | PropertyType          | Optional
HasProperty  | Variable | ConditionSubClassName | LocalizedText[] | PropertyType          | Optional
HasProperty  | Variable | ConditionName         | String          | PropertyType          | Mandatory
HasProperty  | Variable | BranchId              | NodeId          | PropertyType          | Mandatory
HasProperty  | Variable | Retain                | Boolean         | PropertyType          | Mandatory
HasComponent | Variable | EnabledState          | LocalizedText   | TwoStateVariableType  | Mandatory
HasComponent | Variable | Quality               | StatusCode      | ConditionVariableType | Mandatory
HasComponent | Variable | LastSeverity          | UInt16          | ConditionVariableType | Mandatory
HasComponent | Variable | Comment               | LocalizedText   | ConditionVariableType | Mandatory
HasProperty  | Variable | ClientUserId          | String          | PropertyType          | Mandatory
HasComponent | Method   | Disable               | Defined in Clause 5.5.4                 | Mandatory
HasComponent | Method   | Enable                | Defined in Clause 5.5.5                 | Mandatory
HasComponent | Method   | AddComment            | Defined in Clause 5.5.6                 | Mandatory
HasComponent | Method   | ConditionRefresh      | Defined in Clause 5.5.7                 | None
HasComponent | Method   | ConditionRefresh2     | Defined in Clause 5.5.8                 | None

Recommended state names for EnabledState are described in Annex A.

Condition Type | State Variable | False State Name | True State Name
ConditionType  | EnabledState   | Disabled         | Enabled

*/

class QUaTwoStateVariable;
class QUaConditionVariable;

class QUaCondition : public QUaBaseEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaCondition(
		QUaServer *server
	);

	// inherited

	// SourceNode Property identifies the ConditionSource. 
	// If the ConditionSource is not a Node in the AddressSpace, the NodeId is set to NULL. 
	void setSourceNode(const QString &sourceNodeId);

	// children

	// ConditionClassId specifies in which domain this Condition is used.It is the NodeId of the
	// corresponding subtype of BaseConditionClassType. See 5.9 for the definition of ConditionClass
	// and a set of ConditionClasses defined in this standard.When using this Property for filtering,
	// Clients have to specify all individual subtypes of BaseConditionClassType NodeIds. The OfType
	// operator cannot be applied. BaseConditionClassType is used as class whenever a Condition
	// cannot be assigned to a more concrete class.
	QString conditionClassId() const;
	void    setConditionClassId(const QString& conditionClassId);

	// ConditionClassName provides the display name of the subtype of BaseConditionClassType
	QString conditionClassName() const;
	void    setConditionClassName(const QString& conditionClassName);

	// NOTE : optional; not created until one of these methods is called
	// ConditionSubClassId specifies additional class[es] that apply to the Condition.It is the NodeId
	// of the corresponding subtype of BaseConditionClassType.See 5.9.6 for the definition of
	// ConditionClassand a set of ConditionClasses defined in this standard.When using this
	// Property for filtering, Clients have to specify all individual sub types of BaseConditionClassType
	// NodeIds.The OfType operator cannot be applied.The Client specifies a NULL in the filter, to
	// return Conditions where no sub class is applied.When returning Conditions, if this optional field
	// is not available in a Condition, a NULL shall be returned for the field.
	QString conditionSubClassId() const;
	void    setConditionSubClassId(const QString& conditionSubClassId);

	// NOTE : optional; not created until one of these methods is called
	// ConditionSubClassName provides the display name[s] of the ConditionClassType[s] listed in
	// the ConditionSubClassId.
	QString conditionSubClassName() const;
	void    setConditionSubClassName(const QString& conditionSubClassName);

	// ConditionName identifies the Condition instance that the Event originated from. It can be used
	// together with the SourceName in a user display to distinguish between different Condition
	// instances. If a ConditionSource has only one instance of a ConditionType, and the Server has
	// no instance name, the Server shall supply the ConditionType browse name.
	QString conditionName() const;
	void    setConditionName(const QString& conditionName);

	// BranchId is NULL for all Event Notifications that relate to the current state of the Condition
	// instance.If BranchId is not NULL, it identifies a previous state of this Condition instance that
	// still needs attention by an Operator.If the current ConditionBranch is transformed into a
	// previous ConditionBranch then the Server needs to assign a non - NULL BranchId.An initial
	// Event for the branch will generated with the values of the ConditionBranchand the new
	// BranchId.The ConditionBranch can be updated many times before it is no longer needed.When
	// the ConditionBranch no longer requires Operator input the final Event will have Retain set to
	// False.The retain bit on the current Event is True, as long as any ConditionBranches require
	// Operator input.See 4.4 for more information about the need for creatingand maintaining
	// previous ConditionBranchesand Clause B.1 for an example using branches.The BranchId
	// DataType is NodeId although the Server is not required to have ConditionBranches in the
	// Address Space.The use of a NodeId allows the Server to use simple numeric identifiers, strings
	// or arrays of bytes.
	QString branchId() const;
	void    setBranchId(const QString& branchId);

	// Retain when True describes a Condition (or ConditionBranch) as being in a state that is
	// interesting for a Client wishing to synchronize its state with the Server’s state.The logic to
	// determine how this flag is set is Server specific.Typically, all Active Alarms would have the
	// Retain flag set; however, it is also possible for inactive Alarms to have their Retain flag set to
	// TRUE.
	// In normal processing when a Client receives an Event with the Retain flag set to False, the
	// Client should consider this as a ConditionBranch that is no longer of interest, in the case of a
	// “current Alarm display” the ConditionBranch would be removed from the display.
	bool    retain() const;
	void    setRetain(const bool& retain);

	// EnabledState indicates whether the Condition is enabled. EnabledState/Id is True if enabled,
	// False otherwise. EnabledState / TransitionTime defines when the EnabledState last changed.
	// Recommended state names are described in Annex A.
	// A Condition’s EnabledState effects the generation of Event Notificationsand as such results in
	// the following specific behaviour :
	//- When the Condition instance enters the Disabled state, the Retain Property of this
	//	Condition shall be set to False by the Server to indicate to the Client that the Condition
	//	instance is currently not of interest to Clients.This includes all ConditionBranches if any
	//	branches exist.
	//	- When the Condition instance enters the enabled state, the Condition shall be evaluated
	//	  and all of its Properties updated to reflect the current values.If this evaluation causes
	//	  the Retain Property to transition to True for any ConditionBranch, then an Event
	//	  Notification shall be generated for that ConditionBranch.
	//	- The Server may choose to continue to test for a Condition instance while it is Disabled.
	//	  However, no Event Notifications will be generated while the Condition instance is
	//	  disabled.
	//	- For any Condition that exists in the AddressSpace the Attributesand the following
	//	  Variables will continue to have valid values even in the Disabled state; EventId, Event
	//	  Type, Source Node, Source Name, Time, and EnabledState.Other Properties may no
	//	  longer provide current valid values.All Variables that are no longer provided shall return
	//	  a status of Bad_ConditionDisabled.The Event that reports the Disabled state should
	//	  report the Properties as NULL or with a status of Bad_ConditionDisabled.
	// When enabled, changes to the following components shall cause a ConditionType Event
	// Notification :
	//- Quality
	//	- Severity(inherited from BaseEventType)
	//	- Comment
	// This may not be the complete list.Sub - Types may define additional Variables that trigger Event
	// Notifications.In general, changes to Variables of the types TwoStateVariableType or
	// ConditionVariableType trigger Event Notifications.
	QString   enabledStateCurrentStateName() const;
	void      setEnabledStateCurrentStateName(const QString& enabledState);
	bool      enabledStateId() const;
	void      setEnabledStateId(const bool& enabledStateId);
	QDateTime enabledStateTransitionTime() const;
	void      setEnabledStateTransitionTime(const QDateTime& transitionTime);
	QString   enabledStateTrueState() const;
	void      setEnabledStateTrueState(const QString& trueState);
	QString   enabledStateFalseState() const;
	void      setEnabledStateFalseState(const QString& falseState);

	//
	quint32 quality() const;
	void    setQuality(const quint32& quality);

	//
	quint16 lastSeverity() const;
	void    setLastSeverity(const quint16& lastSeverity);

	//
	QString comment() const;
	void    setComment(const QString& comment);

	// 
	QString clientUserId() const;
	void    setClientUserId(const QString& clientUserId);

	// methods

	Q_INVOKABLE void Enable();

	Q_INVOKABLE void Disable();

	Q_INVOKABLE void AddComment(QByteArray EventId, QString Comment);


protected:
	// NodeId
	QUaProperty* getConditionClassId();
	// LocalizedText
	QUaProperty* getConditionClassName();
	// NodeId[]
	QUaProperty* getConditionSubClassId();
	// LocalizedText[]
	QUaProperty* getConditionSubClassName();
	// String
	QUaProperty* getConditionName();
	// NodeId
	QUaProperty* getBranchId();
	// Boolean
	QUaProperty* getRetain();
	// LocalizedText
	QUaTwoStateVariable* getEnabledState();
	// StatusCode
	QUaConditionVariable* getQuality();
	// UInt16
	QUaConditionVariable* getLastSeverity();
	// LocalizedText
	QUaConditionVariable* getComment();
	// String
	QUaProperty* getClientUserId();
};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUACONDITION_H