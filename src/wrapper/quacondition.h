#ifndef QUACONDITION_H
#define QUACONDITION_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

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

	// inherited

	// Overwrite QUaNode::setDisplayName to update conditionName
	virtual void setDisplayName(const QString& displayName) override;

	// Overwrite QUaBaseEvent::setSourceNode to update retained conditions cache
	virtual void setSourceNode(const QUaNodeId& sourceNodeId) override;

	// children

	// TODO : implement condition classes

	QUaNodeId conditionClassId() const;
	void      setConditionClassId(const QUaNodeId& conditionClassId);

	QUaLocalizedText conditionClassName() const;
	void    setConditionClassName(const QUaLocalizedText& conditionClassName);


	// NOTE : optional; not created until one of these methods is called
	QList<QUaNodeId> conditionSubClassId() const;
	void      setConditionSubClassId(const QList<QUaNodeId>& conditionSubClassId);

	// ConditionSubClassName provides the display name[s] of the ConditionClassType[s] listed in
	// the ConditionSubClassId.
	// NOTE : optional; not created until one of these methods is called
	QList<QUaLocalizedText> conditionSubClassName() const;
	void    setConditionSubClassName(const QList<QUaLocalizedText>& conditionSubClassName);

	// to distinguish from which condition the event originated from
	QString conditionName() const;
	void    setConditionName(const QString& conditionName);

	// only set if it is a branch, null if not a branch
	QUaNodeId branchId() const;
	void      setBranchId(const QUaNodeId& branchId);

	// retain is used for for condition sync
	bool    retain() const;
	void    setRetain(const bool& retain);

	QUaLocalizedText enabledStateCurrentStateName() const;
	void             setEnabledStateCurrentStateName(const QUaLocalizedText& enabledState);
	bool             enabledStateId() const;
	void             setEnabledStateId(const bool& enabledStateId);
	QDateTime        enabledStateTransitionTime() const;
	void             setEnabledStateTransitionTime(const QDateTime& transitionTime);
	QUaLocalizedText enabledStateTrueState() const;
	void             setEnabledStateTrueState(const QUaLocalizedText& trueState);
	QUaLocalizedText enabledStateFalseState() const;
	void             setEnabledStateFalseState(const QUaLocalizedText& falseState);
	// helper sets EnabledStateId, EnabledStateCurrentStateName, EnabledStateTransitionTime 
	// and triggers event according to specification
	// NOTE : change of the Enabled state must be normally make by the client through
	//        the use of the Enable() and Disable() methods
	bool      enabled() const;
	void      setEnabled(const bool& enabled);

	// quality of underlying source
	QUaStatusCode quality() const;
	void          setQuality(const QUaStatusCode& quality);

	// 
	quint16 lastSeverity() const;
	// Overwrite to handle lastSeverity automatically
	void    setLastSeverity(const quint16& lastSeverity);
	virtual void setSeverity(const quint16& intSeverity) override;

	QUaLocalizedText comment() const;
	void             setComment(const QUaLocalizedText& comment);

	// identity of who put the last comment
	QString clientUserId() const;
	void    setClientUserId(const QString& clientUserId);

	// methods

	Q_INVOKABLE void Enable(); // TODO : audit events

	Q_INVOKABLE void Disable(); // TODO : audit events

	Q_INVOKABLE void AddComment(QByteArray EventId, QUaLocalizedText Comment); // TODO : audit events

	// branches API

	bool isBranch() const;
	void setIsBranch(const bool& isBranch);

	QUaCondition* mainBranch() const;

	QUaCondition* createBranch(const QUaNodeId& nodeId = QUaNodeId());

	template<typename T>
	T* createBranch(const QUaNodeId& nodeId = QUaNodeId());

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

	// TODO : branchCreated

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
	// NOTE : any change on a ConditionVariable should trigger
	QUaConditionVariable* getQuality();
	// UInt16
	// NOTE : any change on a ConditionVariable should trigger
	QUaConditionVariable* getLastSeverity();
	// LocalizedText
	// NOTE : any change on a ConditionVariable should trigger
	QUaConditionVariable* getComment();
	// String
	QUaProperty* getClientUserId();

	// helpers

	// reimplement to define minimu trigger conditions
	virtual bool shouldTrigger() const override;
	// reimplement to define branch creation / deletion conditions
	virtual bool requiresAttention() const;
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
inline T* QUaCondition::createBranch(const QUaNodeId& nodeId/* = ""*/)
{
	return qobject_cast<T*>(this->createBranch(nodeId));
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
