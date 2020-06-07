#include "quacondition.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>
#include <QUaTwoStateVariable>
#include <QUaConditionVariable>
#include <QUaRefreshStartEvent>
#include <QUaRefreshEndEvent>
#include <QUaRefreshRequiredEvent>

#include "quaserver_anex.h"

QUaCondition::QUaCondition(
	QUaServer *server
) : QUaBaseEvent(server)
{
	m_sourceNode = nullptr;
	m_branchQueueSize = 0;
#ifdef UA_ENABLE_HISTORIZING
	m_historizingBranches = false;
#endif // UA_ENABLE_HISTORIZING
	// force some non-qt datatypes
	//this->getConditionClassId()->setDataType(QMetaType_NodeId);	// NodeId
	//this->getBranchId()->setDataType(QMetaType_NodeId);	        // NodeId
	this->getQuality()->setDataType(QMetaType_StatusCode);	    // StatusCode
	// set default : BaseConditionClassType node id
	// NOTE : ConditionClasses not supported yet
	//this->setConditionClassId(
	//	QUaTypesConverter::nodeIdToQString(
	//		UA_NODEID_NUMERIC(0, UA_NS0ID_BASECONDITIONCLASSTYPE)
	//	)
	//);
	// set default : display name of BaseConditionClassType
	// NOTE : ConditionClasses not supported yet
	//this->setConditionClassName("BaseConditionClass");
	// set default : ConditionType browse name
	this->setConditionName(this->displayName());
	// set default : retain false
	this->setRetain(false);
	// reuse rest of defaults
	this->resetInternals();
}

QUaCondition::~QUaCondition()
{
	// remove destroy connections
	QObject::disconnect(m_sourceDestroyed);
	// NOTE : do not disconnect m_retainedDestroyed, else it will not be called
	
	// delete branches
	std::for_each(m_branches.begin(), m_branches.end(),
	[](QUaConditionBranch* branch) {
		delete branch;
	});
	m_branches.clear();
	// do not trigger if server is being destroyed
	if (m_qUaServer->m_beingDestroyed)
	{
		return;
	}
	// trigger last event so clients can remove from alarm display 
	this->setRetain(false);
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(tr("Condition %1 deleted.")
		.arg(this->displayName().text()));
	this->trigger();
}

void QUaCondition::setDisplayName(const QUaLocalizedText& displayName)
{
	QUaNode::setDisplayName(displayName);
	// also update condition name to be able to diff wrt other conditions
	this->setConditionName(displayName.text());
}

void QUaCondition::setSourceNode(const QUaNodeId& sourceNodeId)
{
	// call base implementation (updates cache m_sourceNodeId)
	QUaBaseEvent::setSourceNode(sourceNodeId);
	// update retained conditions for old source node
	bool isRetained = this->retain();
	if (m_sourceNode)
	{
		m_sourceNode->removeReference({ "HasCondition" , "IsConditionOf" }, this, true);
		// remove destroy connections
		QObject::disconnect(m_sourceDestroyed);
		QObject::disconnect(m_retainedDestroyed);
		// remove from hash
		if (isRetained)
		{	
			Q_ASSERT(m_qUaServer->m_retainedConditions[m_sourceNode].contains(this));
			m_qUaServer->m_retainedConditions[m_sourceNode].remove(this);
		}
	}
	// update source
	if (UA_NodeId_isNull(&m_sourceNodeId))
	{
		return;
	}
	m_sourceNode = QUaNode::getNodeContext(m_sourceNodeId, m_qUaServer->m_server);
	if (m_sourceNode)
	{
		// NOTE : references are RAM hungry and is not worth having them for branches
		m_sourceNode->addReference({ "HasCondition" , "IsConditionOf" }, this, true);
		// add destroy connection
		m_sourceDestroyed = QObject::connect(m_sourceNode, &QObject::destroyed, this,
		[this]() {
			if (m_qUaServer->m_retainedConditions.contains(m_sourceNode))
			{
				m_qUaServer->m_retainedConditions.remove(m_sourceNode);
			}
			m_sourceNode = nullptr;
			// if this node has been removed from library we cannot write to it
			// but C++ instance still exists for a little longer
			QUaNodeId nodeId = this->nodeId();
			if (!m_qUaServer->isNodeIdUsed(nodeId))
			{
				return;
			}
			this->setSourceNode(QUaNodeId());
			this->setSourceName("");
		});
		// add to hash
		if (isRetained)
		{
			// update retained conditions hash for new source node
			Q_ASSERT(!m_qUaServer->m_retainedConditions[m_sourceNode].contains(this));
			m_qUaServer->m_retainedConditions[m_sourceNode].insert(this);
			// add destroy connection
			auto svr = m_qUaServer;
			auto src = m_sourceNode;
			m_retainedDestroyed = QObject::connect(this, &QObject::destroyed,
			[this, svr, src]() {
				if (!svr->m_retainedConditions.contains(src))
				{
					return;
				}
				svr->m_retainedConditions[src].remove(this);
			});
		}
	}
}

// TODO : implement condition classes
QUaNodeId QUaCondition::conditionClassId() const
{
	return const_cast<QUaCondition*>(this)->getConditionClassId()->value<QUaNodeId>();
}
// TODO : implement condition classes
void QUaCondition::setConditionClassId(const QUaNodeId& conditionClassId)
{
	this->getConditionClassId()->setValue(conditionClassId);
}
// TODO : implement condition classes
QUaLocalizedText QUaCondition::conditionClassName() const
{
	return const_cast<QUaCondition*>(this)->getConditionClassName()->value<QUaLocalizedText>();
}
// TODO : implement condition classes
void QUaCondition::setConditionClassName(const QUaLocalizedText& conditionClassName)
{
	this->getConditionClassName()->setValue(conditionClassName);
}

// TODO : implement condition classes
QList<QUaNodeId> QUaCondition::conditionSubClassId() const
{
    return const_cast<QUaCondition*>(this)->getConditionSubClassId()->value<QList<QUaNodeId>>();
}
// TODO : implement condition classes
void QUaCondition::setConditionSubClassId(const QList<QUaNodeId>& conditionSubClassId)
{
	this->getConditionSubClassId()->setValue(conditionSubClassId);
}
// TODO : implement condition classes
QList<QUaLocalizedText> QUaCondition::conditionSubClassName() const
{
	return const_cast<QUaCondition*>(this)->getConditionSubClassName()->value<QList<QUaLocalizedText>>();
}
// TODO : implement condition classes
void QUaCondition::setConditionSubClassName(const QList<QUaLocalizedText>& conditionSubClassName)
{
	this->getConditionSubClassName()->setValue(conditionSubClassName);
}

QString QUaCondition::conditionName() const
{
	return const_cast<QUaCondition*>(this)->getConditionName()->value<QString>();
}

void QUaCondition::setConditionName(const QString& conditionName)
{
	this->getConditionName()->setValue(conditionName);
}

QUaNodeId QUaCondition::branchId() const
{
	return const_cast<QUaCondition*>(this)->getBranchId()->value<QUaNodeId>();
}

void QUaCondition::setBranchId(const QUaNodeId& branchId)
{
	this->getBranchId()->setValue(branchId);
}

bool QUaCondition::retain() const
{
	return const_cast<QUaCondition*>(this)->getRetain()->value<bool>();
}

void QUaCondition::setRetain(const bool& retain)
{
	if (retain == this->retain())
	{
		return;
	}
	// update internal value
	this->getRetain()->setValue(retain);
	// update all branches if this is main branch
	for (auto branch : this->branches())
	{
		branch->setRetain(retain);
	}
	// emit change
	emit this->retainChanged();
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
		// add destroy connection
		auto svr = m_qUaServer;
		auto src = m_sourceNode;
		m_retainedDestroyed = QObject::connect(this, &QObject::destroyed,
		[this, svr, src]() {
			if (!svr->m_retainedConditions.contains(src))
			{
				return;
			}
			svr->m_retainedConditions[src].remove(this);
		});
	}
	else
	{
		Q_ASSERT(m_qUaServer->m_retainedConditions[m_sourceNode].contains(this));
		m_qUaServer->m_retainedConditions[m_sourceNode].remove(this);
		// remove destroy connection
		QObject::disconnect(m_retainedDestroyed);
	}
}

QUaLocalizedText QUaCondition::enabledStateCurrentStateName() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->currentStateName();
}

void QUaCondition::setEnabledStateCurrentStateName(const QUaLocalizedText& enabledState)
{
	this->getEnabledState()->setCurrentStateName(enabledState);
}

bool QUaCondition::enabledStateId() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->id();
}

void QUaCondition::setEnabledStateId(const bool& enabledStateId)
{
	// set enable id
	this->getEnabledState()->setId(enabledStateId);
}

bool QUaCondition::enabled() const
{
	return this->enabledStateId();
}

void QUaCondition::setEnabled(const bool& enabled)
{
	if (enabled == this->enabled())
	{
		return;
	}
	// set enable id
	this->setEnabledStateId(enabled);
	// update current state name
	auto currStateName = enabled ?
		this->enabledStateTrueState() :
		this->enabledStateFalseState();
	this->setEnabledStateCurrentStateName(
		currStateName
	);
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// handle retain
	this->setRetain(enabled ? this->requiresAttention() : false);
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(tr("Condition %1.").arg(currStateName));
	this->trigger();
	// emit qt signal
	if (enabled)
	{
		emit this->conditionEnabled();
	}
	else
	{
		emit this->conditionDisabled();
	}
}

QDateTime QUaCondition::enabledStateTransitionTime() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->transitionTime();
}

void QUaCondition::setEnabledStateTransitionTime(const QDateTime& transitionTime)
{
	this->getEnabledState()->setTransitionTime(transitionTime);
}

QUaLocalizedText QUaCondition::enabledStateTrueState() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->trueState();
}

void QUaCondition::setEnabledStateTrueState(const QUaLocalizedText& trueState)
{
	this->getEnabledState()->setTrueState(trueState);
}

QUaLocalizedText QUaCondition::enabledStateFalseState() const
{
	return const_cast<QUaCondition*>(this)->getEnabledState()->falseState();
}

void QUaCondition::setEnabledStateFalseState(const QUaLocalizedText& falseState)
{
	this->getEnabledState()->setFalseState(falseState);
}

QUaStatusCode QUaCondition::quality() const
{
	return QUaStatus(const_cast<QUaCondition*>(this)->getQuality()->value<QUaStatusCode>());
}

void QUaCondition::setQuality(const QUaStatusCode& quality)
{
	// NOTE : call template base version but should call specialized version
	this->getQuality()->QUaBaseVariable::setValue(quality);
	// any change to comment, severity and quality will cause event.
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(tr("Quality changed to %1.").arg(QString(QUaStatusCode(quality))));
	this->trigger();
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
	if (intSeverity == this->severity())
	{
		return;
	}
	// set last severity before writing new
	this->setLastSeverity(this->severity());
	// set new severity
	QUaBaseEvent::setSeverity(intSeverity);
	// emit change
	emit this->severityChanged();
	// any change to comment, severity and quality will cause event.
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(tr("Severity changed to %1.").arg(intSeverity));
	this->trigger();
}

QUaLocalizedText QUaCondition::comment() const
{
	return const_cast<QUaCondition*>(this)->getComment()->value<QUaLocalizedText>();
}

void QUaCondition::setComment(const QUaLocalizedText& comment)
{
	this->getComment()->QUaBaseVariable::setValue(comment);
	// update user id if applicable
	auto session = this->currentSession();
	if (session)
	{
		this->setClientUserId(session->userName());	
	}
	// any change to comment, severity and quality will cause event.
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(tr("A comment was added."));
	this->trigger();
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
	// change EnabledState to Enabled and trigger event
	this->setEnabled(true);
}

void QUaCondition::Disable()
{
	// check already disabled
	if (!this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONALREADYDISABLED);
		return;
	}
	// change EnabledState to Disabled and trigger event
	this->setEnabled(false);
}

void QUaCondition::AddComment(QByteArray EventId, QUaLocalizedText Comment)
{
	// check if enabled
	if (!this->enabledStateId())
	{
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADCONDITIONDISABLED);
		return;
	}
	// check given EventId matches, if ampty assume method call on this instance
	if (!EventId.isEmpty() && EventId != this->eventId())
	{
		auto branch = this->branchByEventId(EventId);
		if (branch)
		{
			// update user id if applicable
			auto session = this->currentSession();
			QString currentUser;
			if (session)
			{
				currentUser = session->userName();
			}
			branch->setComment(Comment, currentUser);
			return;
		}
		this->setMethodReturnStatusCode(UA_STATUSCODE_BADEVENTIDUNKNOWN);
		return;
	}
	// set comment
	this->setComment(Comment);
	// emit qt signal
	emit this->addedComment(Comment);
}

quint32 QUaCondition::branchQueueSize() const
{
	return m_branchQueueSize;
}

void QUaCondition::setBranchQueueSize(const quint32& branchQueueSize)
{
	m_branchQueueSize = branchQueueSize;
}

#ifdef UA_ENABLE_HISTORIZING
bool QUaCondition::historizingBranches() const
{
	return m_historizingBranches;
}

void QUaCondition::setHistorizingBranches(const bool& historizingBranches)
{
	m_historizingBranches = historizingBranches;
}
#endif // UA_ENABLE_HISTORIZING

QList<QUaConditionBranch*> QUaCondition::branches() const
{
	return m_branches;
}

bool QUaCondition::hasBranches() const
{
	return m_branches.count() > 0;
}

QUaConditionBranch* QUaCondition::branchByEventId(const QByteArray& eventId) const
{
	auto res = std::find_if(m_branches.begin(), m_branches.end(),
	[&eventId](QUaConditionBranch* branch) {
		return branch->eventId() == eventId;
	});
	return res == m_branches.end() ? nullptr : *res;
}

void QUaCondition::removeBranchByEventId(QUaConditionBranch* branch)
{
	m_branches.removeOne(branch);
	if (m_branches.count() > 0)
	{
		return;
	}
	bool requiresAttention = this->requiresAttention();
	this->setMessage(requiresAttention ?
		tr("Condition still requires attention.") :
		tr("Condition no longer of interest.")
	);
	this->setRetain(requiresAttention);
	this->trigger();
}

bool QUaCondition::shouldTrigger() const
{
	bool baseTrigger = QUaBaseEvent::shouldTrigger();
	bool isEnabled   = this->enabledStateId();
	return baseTrigger && isEnabled;
}

bool QUaCondition::requiresAttention() const
{
	return false;
}

void QUaCondition::resetInternals()
{
	// set default : disabled state
	this->setEnabledStateFalseState(tr("Disabled"));
	this->setEnabledStateTrueState(tr("Enabled"));
	this->setEnabledStateCurrentStateName(tr("Disabled"));
	this->getEnabledState()->setId(false);  // (do not trigger event for this enabled state change)
	this->setEnabledStateTransitionTime(this->getEnabledState()->serverTimestamp());
	// set default : good (do not trigger event for this quality change)
	this->getQuality()->QUaBaseVariable::setValue<QUaStatusCode>(QUaStatus::Good);
	// set default : 0
	this->setLastSeverity(0);
	// set default : empty (do not trigger event for this comment change)
	this->getComment()->QUaBaseVariable::setValue<QUaLocalizedText>(tr(""));
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
	Q_UNUSED(sessionContext);
	Q_UNUSED(methodId);
	Q_UNUSED(methodContext);
	Q_UNUSED(objectId);
	Q_UNUSED(objectContext);
	Q_UNUSED(inputSize);
	Q_UNUSED(outputSize);
	Q_UNUSED(output);
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
	Q_UNUSED(sessionContext);
	Q_UNUSED(methodId);
	Q_UNUSED(methodContext);
	Q_UNUSED(objectId);
	Q_UNUSED(objectContext);
	Q_UNUSED(inputSize);
	Q_UNUSED(outputSize);
	Q_UNUSED(output);
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
	// NOTE : clients can still have in their subscriptions node ids that have been deleted
    static UA_NodeId server = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);
    if (!node && !UA_NodeId_equal(&monitoredItem->monitoredNodeId, &server))
	{
		// TODO : log error message
		return;
	}
	QSet<QUaCondition*> conditions;
	if (node)
	{
		// only retained conditions for given monitored item's node
		conditions = srv->m_retainedConditions[node];
	}
	else
	{
		// all retained conditions if monitored item is server object
		std::for_each(srv->m_retainedConditions.begin(), srv->m_retainedConditions.end(),
		[&conditions](const QSet<QUaCondition*>& conds) {
			conditions.unite(conds);
		});
	}
	// NOTE : need to send RefreshStartEvent and RefreshEndEvent for each monitored item even if no retained conditions
	UA_StatusCode retval;
    QString sourceNodeId = node ? QString(node->nodeId()) : QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER));
    QString sourceDisplayName = node ? QString(node->displayName()) : tr("Server");
	/* 1. trigger RefreshStartEvent */
	srv->m_refreshStartEvent->setSourceNode(sourceNodeId);
	srv->m_refreshStartEvent->setSourceName(sourceDisplayName);
	srv->m_refreshStartEvent->setMessage(tr("Start refresh for source %1 [%2].").arg(sourceDisplayName).arg(sourceNodeId));
	retval = QUaServer_Anex::UA_Event_addEventToMonitoredItem(
		srv->m_server, 
		&srv->m_refreshStartEvent->m_nodeId, 
		monitoredItem, 
		nullptr
	);
	Q_ASSERT(retval == UA_STATUSCODE_GOOD);
	/* 2. refresh (see 5.5.7)*/
	for (auto condition : conditions)
	{
		Q_ASSERT(condition->retain());
		if (!condition->shouldTrigger())
		{
			continue;
		}
		retval = QUaServer_Anex::UA_Event_addEventToMonitoredItem(
			srv->m_server, 
			&condition->m_nodeId, 
			monitoredItem, 
			nullptr
		);
		Q_ASSERT(retval == UA_STATUSCODE_GOOD);
		// add branches if any
		for (auto &branch : condition->branches())
		{
			retval = QUaServer_Anex::UA_Event_addEventToMonitoredItem(
				srv->m_server, 
				&condition->m_nodeId, 
				monitoredItem, 
				[branch](const QUaBrowsePath& browsePath) -> QVariant
				{
					return branch->value(browsePath);
				}
			);
			Q_ASSERT(retval == UA_STATUSCODE_GOOD);
		}
	}
	/* 3. trigger RefreshEndEvent*/
	srv->m_refreshEndEvent->setSourceNode(sourceNodeId);
	srv->m_refreshEndEvent->setSourceName(sourceDisplayName);
	srv->m_refreshEndEvent->setMessage(tr("End refresh for source %1 [%2].").arg(sourceDisplayName).arg(sourceNodeId));
	retval = QUaServer_Anex::UA_Event_addEventToMonitoredItem(
		srv->m_server, 
		&srv->m_refreshEndEvent->m_nodeId, 
		monitoredItem, 
		nullptr
	);
	Q_ASSERT(retval == UA_STATUSCODE_GOOD);
}

/****************************************************************************************
*/

// QUaBaseEvent
QUaBrowsePath QUaConditionBranch::EventId     ({ { 0, "EventId"      } }); // [ByteString]
QUaBrowsePath QUaConditionBranch::Message     ({ { 0, "Message"      } }); // [LocalizedText]
QUaBrowsePath QUaConditionBranch::Time        ({ { 0, "Time"         } }); // [UtcTime]
QUaBrowsePath QUaConditionBranch::ClientUserId({ { 0, "ClientUserId" } }); // [String]
// QUaCondition
QUaBrowsePath QUaConditionBranch::BranchId({ { 0, "BranchId" } }); // [NodeId]
QUaBrowsePath QUaConditionBranch::Retain  ({ { 0, "Retain"   } }); // [Boolean]
QUaBrowsePath QUaConditionBranch::EnabledState               ({ { 0, "EnabledState" } }); // [LocalizedText]
QUaBrowsePath QUaConditionBranch::EnabledState_Id            ({ { 0, "EnabledState" },{ 0, "Id"             } }); // [Boolean]
//QUaBrowsePath QUaConditionBranch::EnabledState_FalseState    ({ { 0, "EnabledState" },{ 0, "FalseState"     } }); // [LocalizedText]
//QUaBrowsePath QUaConditionBranch::EnabledState_TrueState     ({ { 0, "EnabledState" },{ 0, "TrueState"      } }); // [LocalizedText]
QUaBrowsePath QUaConditionBranch::EnabledState_TransitionTime({ { 0, "EnabledState" },{ 0, "TransitionTime" } }); // [UtcTime]
QUaBrowsePath QUaConditionBranch::Comment                ({ { 0, "Comment" } }); // [LocalizedText]
QUaBrowsePath QUaConditionBranch::Comment_SourceTimestamp({ { 0, "Comment" },{0, "SourceTimestamp"} }); // [UtcTime]

QUaConditionBranch::QUaConditionBranch(QUaCondition* parent, const QUaNodeId& branchId/* = QUaNodeId()*/)
{
	Q_ASSERT(parent);
	// copy necessary trigger variables
	m_parent = parent;
	// copy tree : start with root
	this->addChildren(parent);
	// set branch id
	this->setBranchId(branchId.isNull() ? QUaNodeId(0, UA_UInt32_random()) : branchId);
	// trigger first event so clients can add branch to alarm display 
	this->trigger();
}

QUaConditionBranch::~QUaConditionBranch()
{
	// trigger last event so clients can remove from alarm display 
	this->setRetain(false);
	this->setMessage(QObject::tr("Branch deleted."));
	// TODO : if delete because out of queue then auto ack and confirm?
	this->trigger();
}

void QUaConditionBranch::deleteLater()
{
	m_parent->m_qUaServer->
	m_changeEventSignaler.execLater([this]() {
		delete this;
	});
}

QVariant QUaConditionBranch::value(const QUaBrowsePath& browsePath) const
{
	uint key = qHash(browsePath);
	// NOTE : possible 
	//Q_ASSERT(m_values.contains(key));
	return m_values.value(key, QVariant());
}

void QUaConditionBranch::setValue(const QUaBrowsePath& browsePath, const QVariant& value)
{
	uint key = qHash(browsePath);
	Q_ASSERT(m_values.contains(key));
	m_values[key] = value;
}

void QUaConditionBranch::trigger()
{
	// set evend id
	this->setEventId(QUaBaseEvent::generateEventId());
	// trigger base condition with special callback
	auto st = QUaServer_Anex::UA_Server_triggerEvent_Modified(
		m_parent->server()->m_server,
		m_parent->m_nodeId,
		m_parent->m_sourceNodeId,
		[this](const QUaBrowsePath& browsePath) -> QVariant
		{
			return this->value(browsePath);
		}
	);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
}

QByteArray QUaConditionBranch::eventId() const
{
	return this->value(QUaConditionBranch::EventId).value<QByteArray>();
}

void QUaConditionBranch::setEventId(const QByteArray& eventId)
{
	this->setValue(QUaConditionBranch::EventId, eventId);
}

void QUaConditionBranch::setMessage(const QUaLocalizedText& message)
{
	this->setValue(QUaConditionBranch::Message, QVariant::fromValue(message));
}

void QUaConditionBranch::setTime(const QDateTime& time)
{
	this->setValue(QUaConditionBranch::Time, time);
}

QUaNodeId QUaConditionBranch::branchId() const
{
	return this->value(QUaConditionBranch::BranchId).value<QUaNodeId>();
}

void QUaConditionBranch::setBranchId(const QUaNodeId& branchId)
{
	this->setValue(QUaConditionBranch::BranchId, QVariant::fromValue(branchId));
}

void QUaConditionBranch::setRetain(const bool& retain)
{
	this->setValue(QUaConditionBranch::Retain, retain);
}

void QUaConditionBranch::setComment(const QUaLocalizedText& comment, const QString& currentUser)
{
	auto time = QDateTime::currentDateTimeUtc();
	// set comment
	this->setValue(QUaConditionBranch::Comment, QVariant::fromValue(comment));
	this->setValue(QUaConditionBranch::Comment_SourceTimestamp, time);
	// update user that set comment
	this->setValue(QUaConditionBranch::ClientUserId, currentUser);
	// any change to comment, severity and quality will cause event.	
	// NOTE : do not set time
	this->trigger();
}

QSet<QUaQualifiedName> ignoreSet = QSet<QUaQualifiedName>()
<< QUaQualifiedName( 0, "FalseState" )
<< QUaQualifiedName( 0, "TrueState"  );

void QUaConditionBranch::addChildren(QUaNode* node, const QUaBrowsePath& browsePath/* = QUaBrowsePath()*/)
{
	// leaves
	for (auto prop : node->browseChildren<QUaProperty>())
	{
		auto browseName = prop->browseName();
		if (ignoreSet.contains(browseName))
		{
			continue;
		}
		auto newBrowsePath = browsePath + QUaBrowsePath() << browseName;
		//qDebug() << QUaQualifiedName::reduceName(newBrowsePath);
		uint key = qHash(newBrowsePath);
		Q_ASSERT(!m_values.contains(key));
		m_values[key] = prop->value();
		// no children
	}
	// variables
	for (auto var : node->browseChildren<QUaBaseDataVariable>())
	{
		auto browseName = var->browseName();
		if (ignoreSet.contains(browseName))
		{
			continue;
		}
		auto newBrowsePath = browsePath + QUaBrowsePath() << browseName;
		//qDebug() << QUaQualifiedName::reduceName(newBrowsePath);
		uint key = qHash(newBrowsePath);
		Q_ASSERT(!m_values.contains(key));
		m_values[key] = var->value();
		this->addChildren(var, newBrowsePath);
	}
}

bool QUaConditionBranch::requiresAttention() const
{
	return false;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
