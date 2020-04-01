#include "quacondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaTwoStateVariable>
#include <QUaConditionVariable>
#include <QUaRefreshStartEvent>
#include <QUaRefreshEndEvent>

#include "quaserver_anex.h"

QUaCondition::QUaCondition(
	QUaServer *server
) : QUaBaseEvent(server)
{
	m_sourceNode = nullptr;
	// set default : BaseConditionClassType node id
	// NOTE : ConditionClasses not supported yet
	this->setConditionClassId(
		QUaTypesConverter::nodeIdToQString(
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASECONDITIONCLASSTYPE)
		)
	);
	// set default : display name of BaseConditionClassType
	// NOTE : ConditionClasses not supported yet
	this->setConditionClassName("BaseConditionClass");
	// set default : ConditionType browse name
	this->setConditionName(this->typeDefinitionBrowseName());
	// set default : retain false
	this->setRetain(false);
	// set default : disabled state
	this->setEnabledStateFalseState("Disabled");
	this->setEnabledStateTrueState("Enabled");
	this->setEnabledStateCurrentStateName("Disabled");
	this->setEnabledStateId(false);
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// reuse rest of defaults
	this->resetInternals();
}

void QUaCondition::setSourceNode(const QString& sourceNodeId)
{
	// call base implementation (updates cache)
	QUaBaseEvent::setSourceNode(sourceNodeId);
	// update retained conditions for old source node
	bool isRetained = this->retain();
	if (m_sourceNode && isRetained)
	{
		Q_ASSERT(m_qUaServer->m_retainedConditions[m_sourceNode].contains(this));
		m_qUaServer->m_retainedConditions[m_sourceNode].remove(this);
	}
	// update source
	m_sourceNode = QUaNode::getNodeContext(m_nodeIdOriginator, m_qUaServer);
	if (m_sourceNode && isRetained)
	{
		// update retained conditions for new source node
		Q_ASSERT(!m_qUaServer->m_retainedConditions[m_sourceNode].contains(this));
		m_qUaServer->m_retainedConditions[m_sourceNode].insert(this);
	}
}

QString QUaCondition::conditionClassId() const
{
	return const_cast<QUaCondition*>(this)->getConditionClassId()->value().toString();
}

void QUaCondition::setConditionClassId(const QString& conditionClassId)
{
	this->getConditionClassId()->setValue(
		conditionClassId,
		QUaStatus::Good,
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

QString QUaCondition::conditionClassName() const
{
	return const_cast<QUaCondition*>(this)->getConditionClassName()->value().toString();
}

void QUaCondition::setConditionClassName(const QString& conditionClassName)
{
	this->getConditionClassName()->setValue(conditionClassName);
}

QString QUaCondition::conditionSubClassId() const
{
	return const_cast<QUaCondition*>(this)->getConditionSubClassId()->value().toString();
}

void QUaCondition::setConditionSubClassId(const QString& conditionSubClassId)
{
	this->getConditionSubClassId()->setValue(
		conditionSubClassId,
		QUaStatusCode(),
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

QString QUaCondition::conditionSubClassName() const
{
	return const_cast<QUaCondition*>(this)->getConditionSubClassName()->value().toString();
}

void QUaCondition::setConditionSubClassName(const QString& conditionSubClassName)
{
	this->getConditionSubClassName()->setValue(conditionSubClassName);
}

QString QUaCondition::conditionName() const
{
	return const_cast<QUaCondition*>(this)->getConditionName()->value().toString();
}

void QUaCondition::setConditionName(const QString& conditionName)
{
	this->getConditionName()->setValue(conditionName);
}

QString QUaCondition::branchId() const
{
	return const_cast<QUaCondition*>(this)->getBranchId()->value().toString();
}

void QUaCondition::setBranchId(const QString& branchId)
{
	this->getBranchId()->setValue(
		branchId,
		QUaStatusCode(),
		QDateTime(),
		QDateTime(),
		METATYPE_NODEID
	);
}

bool QUaCondition::retain() const
{
	return const_cast<QUaCondition*>(this)->getRetain()->value().toBool();
}

void QUaCondition::setRetain(const bool& retain)
{
	if (retain == this->retain())
	{
		return;
	}
	// update internal value
	this->getRetain()->setValue(retain);
	// update source
	if (!m_sourceNode)
	{
		return;
	}
	// update retained conditions for source node
	if (retain)
	{
		Q_ASSERT(!m_qUaServer->m_retainedConditions[m_sourceNode].contains(this));
		m_qUaServer->m_retainedConditions[m_sourceNode].insert(this);
	}
	else
	{
		Q_ASSERT(m_qUaServer->m_retainedConditions[m_sourceNode].contains(this));
		m_qUaServer->m_retainedConditions[m_sourceNode].remove(this);
	}
}

QString QUaCondition::enabledStateCurrentStateName() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->currentStateName();
}

void QUaCondition::setEnabledStateCurrentStateName(const QString& enabledState)
{
	this->getEnabledState()->setCurrentStateName(enabledState);
}

bool QUaCondition::enabledStateId() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->id();
}

void QUaCondition::setEnabledStateId(const bool& enabledStateId)
{
	this->getEnabledState()->setId(enabledStateId);
}

QDateTime QUaCondition::enabledStateTransitionTime() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->transitionTime();
}

void QUaCondition::setEnabledStateTransitionTime(const QDateTime& transitionTime)
{
	this->getEnabledState()->setTransitionTime(transitionTime);
}

QString QUaCondition::enabledStateTrueState() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->trueState();
}

void QUaCondition::setEnabledStateTrueState(const QString& trueState)
{
	this->getEnabledState()->setTrueState(trueState);
}

QString QUaCondition::enabledStateFalseState() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->falseState();
}

void QUaCondition::setEnabledStateFalseState(const QString& falseState)
{
	this->getEnabledState()->setFalseState(falseState);
}

QUaStatus QUaCondition::quality() const
{
	return QUaStatus(const_cast<QUaCondition*>(this)->getQuality()->value().toUInt());
}

void QUaCondition::setQuality(const QUaStatus& quality)
{
	this->getQuality()->setValue(
		static_cast<quint32>(quality),
		QUaStatus::Good,
		QDateTime(),
		QDateTime(),
		METATYPE_STATUSCODE
	);
}

quint16 QUaCondition::lastSeverity() const
{
	return const_cast<QUaCondition*>(this)->getLastSeverity()->value().toUInt();
}

void QUaCondition::setLastSeverity(const quint16& lastSeverity)
{
	this->getLastSeverity()->setValue(lastSeverity);
}

void QUaCondition::setSeverity(const quint16& intSeverity)
{
	// set last severity before writing new
	this->setLastSeverity(this->severity());
	// set new severity
	QUaBaseEvent::setSeverity(intSeverity);
}

QString QUaCondition::comment() const
{
	return const_cast<QUaCondition*>(this)->getComment()->value().toString();
}

void QUaCondition::setComment(const QString& comment)
{
	this->getComment()->setValue(comment);
	auto session = this->currentSession();
	if (!session)
	{
		return;
	}
	this->setClientUserId(session->userName());
}

QString QUaCondition::clientUserId() const
{
	return const_cast<QUaCondition*>(this)->getClientUserId()->value().toString();
}

void QUaCondition::setClientUserId(const QString& clientUserId)
{
	this->getClientUserId()->setValue(clientUserId);
}

void QUaCondition::Enable()
{
	// check already enabled
	if (this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONALREADYENABLED);
		return;
	}
	// change EnabledState to Enabled
	this->setEnabledStateCurrentStateName("Enabled");
	this->setEnabledStateId(true);
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setSeverity(0);
	this->setMessage(tr("Condition enabled"));
	this->setTime(time);
	this->setReceiveTime(time);
	this->trigger();
	// emit qt signal
	emit this->enabled();
}

void QUaCondition::Disable()
{
	// check already disabled
	if (!this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONALREADYDISABLED);
		return;
	}
	// change EnabledState to Disabled
	this->setEnabledStateCurrentStateName("Disabled");
	this->setEnabledStateId(false);
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setSeverity(0);
	this->setMessage(tr("Condition disabled"));
	this->setTime(time);
	this->setReceiveTime(time);
	this->trigger();
	// emit qt signal
	emit this->disabled();
}

void QUaCondition::AddComment(QByteArray EventId, QString Comment)
{
	// check if enabled
	if (!this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONDISABLED);
		return;
	}
	// check given event id matches last event id
	if (EventId != this->eventId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADEVENTIDUNKNOWN);
		return;
	}
	// set last comment
	this->setComment(Comment);
	// set message
	this->setMessage(tr("A comment was added"));
	// emit qt signal
	emit this->addedComment(Comment);
}

QUaCondition* QUaCondition::createBranch(const QString& strNodeId/* = ""*/)
{
	auto branch = qobject_cast<QUaCondition*>(this->cloneNode(nullptr, strNodeId));
	Q_ASSERT(branch);
	this->addReference({ "HasBranch", "IsBranchOf" }, branch);
	return branch;
}

void QUaCondition::resetInternals()
{
	// set default : good
	this->setQuality(QUaStatus::Good);
	// set default : 0
	this->setLastSeverity(0);
	// set default : empty
	this->setComment("Condition has been reset");
}

QUaProperty* QUaCondition::getConditionClassId()
{
	return this->browseChild<QUaProperty>("ConditionClassId");
}

QUaProperty* QUaCondition::getConditionClassName()
{
	return this->browseChild<QUaProperty>("ConditionClassName");
}

QUaProperty* QUaCondition::getConditionSubClassId()
{
	return this->browseChild<QUaProperty>("ConditionSubClassId", true);
}

QUaProperty* QUaCondition::getConditionSubClassName()
{
	return this->browseChild<QUaProperty>("ConditionSubClassName", true);
}

QUaProperty* QUaCondition::getConditionName()
{
	return this->browseChild<QUaProperty>("ConditionName");
}

QUaProperty* QUaCondition::getBranchId()
{
	return this->browseChild<QUaProperty>("BranchId");
}

QUaProperty* QUaCondition::getRetain()
{
	return this->browseChild<QUaProperty>("Retain");
}

QUaTwoStateVariable* QUaCondition::getEnabledState()
{
	return this->browseChild<QUaTwoStateVariable>("EnabledState");
}

QUaConditionVariable* QUaCondition::getQuality()
{
	return this->browseChild<QUaConditionVariable>("Quality");
}

QUaConditionVariable* QUaCondition::getLastSeverity()
{
	return this->browseChild<QUaConditionVariable>("LastSeverity");
}

QUaConditionVariable* QUaCondition::getComment()
{
	return this->browseChild<QUaConditionVariable>("Comment");
}

QUaProperty* QUaCondition::getClientUserId()
{
	return this->browseChild<QUaProperty>("ClientUserId");
}

UA_StatusCode QUaCondition::ConditionRefresh(
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
)
{
	QUaServer* srv = QUaServer::getServerNodeContext(server);
	Q_ASSERT(srv);
	auto time = QDateTime::currentDateTimeUtc();
	srv->m_refreshStartEvent->setEventId(QUaBaseEvent::generateEventId());
	srv->m_refreshStartEvent->setTime(time);
	srv->m_refreshStartEvent->setReceiveTime(time);
	srv->m_refreshEndEvent->setEventId(QUaBaseEvent::generateEventId());
	srv->m_refreshEndEvent->setTime(time);
	srv->m_refreshEndEvent->setReceiveTime(time);
	/* Check if valid subscriptionId */
	UA_Session* session = UA_Server_getSessionById(server, sessionId);
	UA_Subscription* subscription =
		UA_Session_getSubscriptionById(session, *((UA_UInt32*)input[0].data));
	if (!subscription)
		return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
	/* process each monitoredItem in the subscription */
	UA_StatusCode retval;
	UA_MonitoredItem* monitoredItem = NULL;
	LIST_FOREACH(monitoredItem, &subscription->monitoredItems, listEntry) 
	{
		QUaCondition::processMonitoredItem(monitoredItem, srv);
	}
	return UA_STATUSCODE_GOOD;
}

UA_StatusCode QUaCondition::ConditionRefresh2(
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
)
{
	QUaServer* srv = QUaServer::getServerNodeContext(server);
	Q_ASSERT(srv);
	auto time = QDateTime::currentDateTimeUtc();
	srv->m_refreshStartEvent->setEventId(QUaBaseEvent::generateEventId());
	srv->m_refreshStartEvent->setTime(time);
	srv->m_refreshStartEvent->setReceiveTime(time);
	srv->m_refreshEndEvent->setEventId(QUaBaseEvent::generateEventId());
	srv->m_refreshEndEvent->setTime(time);
	srv->m_refreshEndEvent->setReceiveTime(time);
	/* Check if valid subscriptionId */
	UA_Session* session = UA_Server_getSessionById(server, sessionId);
	UA_Subscription* subscription =
		UA_Session_getSubscriptionById(session, *((UA_UInt32*)input[0].data));
	if (!subscription)
		return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
	/* Process monitored item */
	UA_MonitoredItem* monitoredItem =
		UA_Subscription_getMonitoredItem(subscription, *((UA_UInt32*)input[1].data));
	if (!monitoredItem)
		return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;

	QUaCondition::processMonitoredItem(monitoredItem, srv);
	return UA_STATUSCODE_GOOD;
}

void QUaCondition::processMonitoredItem(UA_MonitoredItem* monitoredItem, QUaServer* srv)
{
	QUaNode* node = QUaNode::getNodeContext(monitoredItem->monitoredNodeId, srv->m_server);
	Q_ASSERT(node || UA_NodeId_equal(&monitoredItem->monitoredNodeId, &UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)));
	if (node && srv->m_retainedConditions[node].count() == 0)
	{
		return;
	}
	QSet<QUaCondition*> conditions;
	if (node)
	{
		conditions = srv->m_retainedConditions[node];
	}
	else
	{
		std::for_each(srv->m_retainedConditions.begin(), srv->m_retainedConditions.end(),
		[&conditions](const QSet<QUaCondition*>& conds) {
			conditions.unite(conds);
		});
	}
	QString sourceNodeId = node ? node->nodeId() : QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER));
	QString sourceDisplayName = node ? node->displayName() : tr("Server");
	/* 1. trigger RefreshStartEvent */
	srv->m_refreshStartEvent->setSourceNode(sourceNodeId);
	srv->m_refreshStartEvent->setSourceName(sourceDisplayName);
	srv->m_refreshStartEvent->triggerRaw();
	/* 2. refresh (see 5.5.7)*/
	for (auto condition : conditions)
	{
		Q_ASSERT(condition->retain());
		auto retval = UA_Event_addEventToMonitoredItem(srv->m_server, &condition->m_nodeId, monitoredItem);
		Q_ASSERT(retval == UA_STATUSCODE_GOOD);
	}
	/* 3. trigger RefreshEndEvent*/
	srv->m_refreshEndEvent->setSourceNode(sourceNodeId);
	srv->m_refreshEndEvent->setSourceName(sourceDisplayName);
	srv->m_refreshEndEvent->triggerRaw();
}




#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS