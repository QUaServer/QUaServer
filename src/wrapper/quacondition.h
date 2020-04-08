#ifndef QUACONDITION_H
#define QUACONDITION_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

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

Events are only generated for Conditions that have their Retain field set to True and for the
initial transition of the Retain field from True to False.

The NodeId of the Condition instance is used as ConditionId. It is not explicitly modelled as a
component of the ConditionType. However, it can be requested with a
SimpleAttributeOperand (see Table 10) in the SelectClause of the EventFilter

The ConditionSource is element which a specific Condition is based upon or related to
Typically, it will be a Variable representing a process tag (e.g. FIC101) or an Object representing
a device or subsystem. In Events generated for Conditions, the SourceNode Property (inherited 
from the BaseEventType) will contain the NodeId of the ConditionSource.

In systems where Conditions are not available as instances, the ConditionSource can reference
the ConditionTypes instead.

Clients can locate Conditions by first browsing for ConditionSources following HasEventSource
References (including sub-types like the HasNotifier Reference) and then browsing for
HasCondition References from all target Nodes of the discovered References.

*/

class QUaTwoStateVariable;
class QUaConditionVariable;

class QUaCondition : public QUaBaseEvent
{
    Q_OBJECT

	// NOTE : any property that has special logic should be made a writable metaproperty to deserialize properly
	Q_PROPERTY(QString sourceNode READ sourceNode WRITE setSourceNode)
	Q_PROPERTY(bool    retain     READ retain     WRITE setRetain    )
	Q_PROPERTY(quint16 severity   READ severity   WRITE setSeverity  )

	Q_PROPERTY(bool    isBranch   READ isBranch   WRITE setIsBranch  )

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaCondition(
		QUaServer *server
	);
	~QUaCondition();

	bool isBranch() const;
	void setIsBranch(const bool& isBranch);

	// inherited

	// Overwrite QUaNode displayName to update conditionName
	virtual void setDisplayName(const QString& displayName) override;

	// SourceNode Property identifies the ConditionSource. 
	// If the ConditionSource is not a Node in the AddressSpace, the NodeId is set to NULL. 
	virtual void setSourceNode(const QString& sourceNodeId) override;

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
	// previous ConditionBranch then the Server needs to assign a non - NULL BranchId. An initial
	// Event for the branch will generated with the values of the ConditionBranch and the new
	// BranchId. The ConditionBranch can be updated many times before it is no longer needed. When
	// the ConditionBranch no longer requires Operator input the final Event will have Retain set to
	// False. The retain bit on the current Event is True, as long as any ConditionBranches require
	// Operator input. See 4.4 for more information about the need for creating and maintaining
	// previous ConditionBranches and Clause B.1 for an example using branches. The BranchId
	// DataType is NodeId although the Server is not required to have ConditionBranches in the
	// Address Space. The use of a NodeId allows the Server to use simple numeric identifiers, strings
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
	// This may not be the complete list. Sub-Types may define additional Variables that trigger 
	// Event Notifications. In general, changes to Variables of the types TwoStateVariableType or
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
	// helper sets EnabledStateId, EnabledStateCurrentStateName, EnabledStateTransitionTime 
	// and triggers event according to specification
	// NOTE : change of the Enabled state must be normally make by the client through
	//        the use of the Enable() and Disable() methods
	bool      enabled() const;
	void      setEnabled(const bool& enabled);

	// Quality reveals the status of process values or other resources that this Condition instance is
	// based upon. Values for the Quality can be any of the OPC StatusCodes defined in OPC Part 8 as well 
	// as Good, Uncertainand Bad as defined in OPC Part 4. These StatusCodes are similar to but slightly 
	// more generic than the description of data quality in the various field bus specifications.
	// It is the responsibility of the Server to map internal status information to these codes.
	// A Server that supports no quality information shall return Good.
	// This quality can also reflect the communication status associated with the system that this value
	// or resource is based on and from which this Alarm was received. For communication errors to
	// the underlying system, especially those that result in some unavailable Event fields, the quality
	// shall be Bad_NoCommunication error.
	QUaStatus quality() const;
	void      setQuality(const QUaStatus& quality);

	// LastSeverity provides the previous severity of the ConditionBranch. Initially this Variable
	// contains a zero value; it will return a value only after a severity change.The new severity is
	// supplied via the Severity Property, which is inherited from the BaseEventType.
	// (Events are only generated for Conditions that have their Retain field set to True)
	quint16 lastSeverity() const;
	void    setLastSeverity(const quint16& lastSeverity);
	virtual void setSeverity(const quint16& intSeverity) override;

	// Comment contains the last comment provided for a certain state (ConditionBranch). It may have
	// been provided by an AddComment Method, some other Method or in some other manner.The
	// initial value of this Variable is NULL, unless it is provided in some other manner.If a Method
	// provides as an option the ability to set a Comment, then the value of this Variable is reset to
	// NULL if an optional comment is not provided.
	QString comment() const;
	void    setComment(const QString& comment);

	// ClientUserId is related to the Comment field and contains the identity of the user who inserted
	// the most recent Comment.The logic to obtain the ClientUserId is defined in OPC Part 5.
	// (The ClientUserId is obtained directly or indirectly from the UserIdentityToken passed by the Client
	// in the ActivateSession Service call)
	QString clientUserId() const;
	void    setClientUserId(const QString& clientUserId);

	// methods

	// The Enable Method is used to change a Condition instance to the enabled state. Normally, the
	// NodeId of the object instance as the ObjectId is passed to the Call Service.However, some
	// Servers do not expose Condition instances in the AddressSpace.Therefore, all Servers shall
	// allow Clients to call the Enable Method by specifying ConditionId as the ObjectId.The Method
	// cannot be called with an ObjectId of the ConditionType Node.If the Condition instance is not
	// exposed, then it may be difficult for a Client to determine the ConditionId for a disabled
	// Condition.
	Q_INVOKABLE void Enable(); // TODO : audit events

	// The Disable Method is used to change a Condition instance to the Disabled state. Normally, the
	// NodeId of the object instance as the ObjectId is passed to the Call Service.However, some
	// Servers do not expose Condition instances in the AddressSpace.Therefore, all Servers shall
	// allow Clients to call the Disable Method by specifying ConditionId as the ObjectId.The Method
	// cannot be called with an ObjectId of the ConditionType Node.
	Q_INVOKABLE void Disable(); // TODO : audit events

	// The AddComment Method is used to apply a comment to a specific state of a Condition instance.
	// Normally, the NodeId of the Object instance is passed as the ObjectId to the Call Service.
	// However, some Servers do not expose Condition instances in the AddressSpace.Therefore, all
	// Servers shall also allow Clients to call the AddComment Method by specifying ConditionId as
	// the ObjectId.The Method cannot be called with an ObjectId of the ConditionType Node.
	Q_INVOKABLE void AddComment(QByteArray EventId, QString Comment); // TODO : audit events, branches

	// branches API

	// In certain environments , it is required to acknowledge both the transition into Active state
	// and the transition into an inactive state. Systems with strict safety rules sometimes require 
	// that every transition into Active state has to be acknowledged. In situations where state changes 
	// occur in short succession there can be multiple unacknowledged states and the Server has to maintain 
	// ConditionBranches for all previous unacknowledged states.
	// These branches will be deleted after they have been acknowledged or if they reached their final state.

	// Multiple ConditionBranches can also be used for other use cases where snapshots of previous
	// states of a Condition require additional actions.

	// When the state represented by a ConditionBranch does not need further attention, a final 
	// Event Notification for this branch will have the Retain Property set to False.

	QUaCondition* createBranch(const QString& strNodeId = "");

	template<typename T>
	T* createBranch(const QString& strNodeId = "");

	// get all branches
	QList<QUaCondition*> branches() const;

	template<typename T>
	QList<T*> branches() const;

	// get branch by EventId (OPC UA Methods can be called by EventId)
	QUaCondition* branchByEventId(const QByteArray& EventId) const;

	template<typename T>
	T* branchByEventId(const QByteArray& EventId) const;

signals:
	void enabled();

	void disabled();

	void addedComment(const QString& comment);

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

	// helpers

	// reimplement to define minimu trigger conditions
	virtual bool shouldTrigger() const override;
	// reimplement to define branch delete conditions
	virtual bool canDeleteBranch() const;
	// reimplement to reset type internals (QUaAlarmCondition::Reset)
	virtual void resetInternals();

private:
	QUaNode * m_sourceNode;
	QMetaObject::Connection m_sourceDestroyed;
	QMetaObject::Connection m_retainedDestroyed;

	bool m_isBranch;
	QSet<QUaCondition*> m_branches;

	//
	static UA_StatusCode ConditionRefresh(
		UA_Server*        server,
		const UA_NodeId*  sessionId,
		void*             sessionContext,
		const UA_NodeId*  methodId,
		void*             methodContext,
		const UA_NodeId*  objectId,
		void*             objectContext,
		size_t            inputSize,
		const UA_Variant* input,
		size_t            outputSize,
		UA_Variant*       output
	);
	//
	static UA_StatusCode ConditionRefresh2(
		UA_Server*        server,
		const UA_NodeId*  sessionId,
		void*             sessionContext,
		const UA_NodeId*  methodId,
		void*             methodContext,
		const UA_NodeId*  objectId,
		void*             objectContext,
		size_t            inputSize,
		const UA_Variant* input,
		size_t            outputSize,
		UA_Variant*       output
	);

	// helpers
	static void processMonitoredItem(
		UA_MonitoredItem* monitoredItem,
		QUaServer* svr
	);

};

template<typename T>
inline T* QUaCondition::createBranch(const QString& strNodeId/* = ""*/)
{
	return qobject_cast<T*>(this->createBranch(strNodeId));
}

template<typename T>
inline QList<T*> QUaCondition::branches() const
{
	QList<T*> retBranches;
	for (auto branch : this->branches())
	{
		auto specialized = qobject_cast<T*>(branch);
		if (!specialized)
		{
			continue;
		}
		retBranches << specialized;
	}
	return retBranches;
}

template<typename T>
inline T* QUaCondition::branchByEventId(const QByteArray& EventId) const
{
	return qobject_cast<T*>(this->branchByEventId(EventId));
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUACONDITION_H
