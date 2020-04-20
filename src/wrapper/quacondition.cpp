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
	m_isBranch   = false;
	// force some non-qt datatypes
	this->getConditionClassId()->setDataType(QMetaType_NodeId);	// NodeId
	this->getBranchId()->setDataType(QMetaType_NodeId);	        // NodeId
	this->getQuality()->setDataType(QMetaType_StatusCode);	    // StatusCode
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
	
	// trigger last event so clients can remove branch from alarm display 
	this->setRetain(false);
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(tr("%1 %2 deleted.")
		.arg(this->isBranch() ? tr("Branch") : tr("Condition"))
		.arg(this->displayName().text()));
	this->trigger();
	// if branch remove qt parent so marked as not in browse tree, no model change event
	if (this->isBranch())
	{
		this->setParent(nullptr);
	}
}

bool QUaCondition::isBranch() const
{
	return m_isBranch;
}

void QUaCondition::setIsBranch(const bool& isBranch)
{
	m_isBranch = isBranch;
}

QUaCondition* QUaCondition::mainBranch() const
{
	Q_ASSERT_X(this->isBranch(), "QUaCondition::mainBranch", "Only brached have a main branch");
	return qobject_cast<QUaCondition*>(this->parent());
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
		// remove reference
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
	m_sourceNode = QUaNode::getNodeContext(m_sourceNodeId, m_qUaServer);
	if (m_sourceNode)
	{
		// add reference
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
	if (!this->isBranch())
	{
		for (auto branch : this->branches())
		{
			branch->setRetain(retain);
		}
	}
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
	// handle branches
	if (!this->isBranch())
	{
		for (auto branch : this->branches())
		{
			branch->setEnabled(enabled);
		}
	}
	// trigger event
	auto time = QDateTime::currentDateTimeUtc();
	this->setTime(time);
	this->setReceiveTime(time);
	this->setMessage(tr("Condition %1.").arg(currStateName));
	this->trigger();
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
	// set last severity before writing new
	this->setLastSeverity(this->severity());
	// set new severity
	QUaBaseEvent::setSeverity(intSeverity);
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
	// emit qt signal
	emit this->conditionEnabled();
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
	// emit qt signal
	emit this->conditionDisabled();
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
			branch->AddComment(EventId, Comment);
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

QUaCondition* QUaCondition::createBranch(const QUaNodeId& nodeId/* = ""*/)
{
	// Are branches of branches supported?
	Q_ASSERT_X(!this->isBranch(), "QUaCondition::createBranch", "Only main branch can created branches");
	if (this->isBranch())
	{
		return nullptr;
	}
	// branches have to hierarchical parent, so browse name is not really relevant
	QUaQualifiedName browseName = this->browseName();
	browseName.setName(tr("%1_branch")
		.arg(browseName.name())
	);
	// NOTE : must pass nullptr as parent because cloneNode uses the
	//        serialization API which recurses all children and if this
	//        has new branch as children, when deserializing it will deserialize
	//        the branch with a branch which will have a branch and so on...
	auto branch = qobject_cast<QUaCondition*>(this->cloneNode(nullptr, browseName, nodeId));
	branch->setParent(this); // set qt parent for memory management
	branch->setIsBranch(true);
	branch->setBranchId(branch->nodeId());
	QUaLocalizedText displayName = tr("%1_branch_%2").arg(this->browseName().name()).arg(branch->nodeId());
	// overwritten setDisplayName also sets condition name
	branch->setDisplayName(displayName);
	// A branch is an independent copy of the condition instance state that can change, not typically 
	// not visible in the Address Space
	// NOTE : in this implementation I decided to expose to the address space through a non-hierarchical
	// reference because it is easier for debugging/development puposes. Maybe will change in the future.
	this->addReference({ "HasBranch", "IsBranchOf" }, branch);
	Q_ASSERT(!m_branches.contains(branch));
	m_branches << branch;
	// remove from internal list if deleted
	QObject::connect(branch, &QObject::destroyed, this,
	[this, branch]() {
		m_branches.remove(branch);
		// update retain flag on main branch is there are no more branches
		if (this->hasBranches())
		{
			return;
		}
		// TODO : implement better, maybe use inheritance?
		QString strMessage;
		if (this->requiresAttention())
		{
			strMessage = this->message().text();
			strMessage.remove(" Has branches.");
		}
		else
		{
			strMessage = tr("Condition no longer of interest.");
		}
		this->setMessage(strMessage);
		this->setRetain(this->requiresAttention());
		this->trigger();
	});
	// trigger first event so clients can add branch to alarm display 
	branch->setMessage(tr("Previous state requires attention. Branch %1 created.").arg(displayName.text()));
	branch->trigger();
	return branch;
}

QList<QUaCondition*> QUaCondition::branches() const
{
	return m_branches.toList();
}

bool QUaCondition::hasBranches() const
{
	Q_ASSERT_X(!this->isBranch(), "QUaCondition::hasBranches", "Only main branch can have branches");
	return this->branches().count() > 0;
}

QUaCondition* QUaCondition::branchByEventId(const QByteArray& EventId) const
{
	auto res = std::find_if(m_branches.begin(), m_branches.end(),
	[&EventId](QUaCondition* branch) {
		return branch->eventId() == EventId;
	});
	return res == m_branches.end() ? nullptr : *res;
}

bool QUaCondition::shouldTrigger() const
{
	bool baseTrigger = QUaBaseEvent::shouldTrigger();
	bool isEnabled   = this->enabledStateId();
	// branch can only trigger if main condition is enabled
	if (this->isBranch())
	{
		auto mainBranch = this->mainBranch();
		Q_ASSERT(mainBranch);
		isEnabled = mainBranch->enabledStateId();;
	}
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
	if (!node && !UA_NodeId_equal(&monitoredItem->monitoredNodeId, &UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)))
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
	QString sourceNodeId = node ? node->nodeId() : QUaTypesConverter::nodeIdToQString(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER));
	QString sourceDisplayName = node ? node->displayName() : tr("Server");
	/* 1. trigger RefreshStartEvent */
	srv->m_refreshStartEvent->setSourceNode(sourceNodeId);
	srv->m_refreshStartEvent->setSourceName(sourceDisplayName);
	srv->m_refreshStartEvent->setMessage(tr("Start refresh for source %1 [%2].").arg(sourceDisplayName).arg(sourceNodeId));
	retval = UA_Event_addEventToMonitoredItem(srv->m_server, &srv->m_refreshStartEvent->m_nodeId, monitoredItem);
	Q_ASSERT(retval == UA_STATUSCODE_GOOD);
	/* 2. refresh (see 5.5.7)*/
	for (auto condition : conditions)
	{
		Q_ASSERT(condition->retain());
		if (!condition->shouldTrigger())
		{
			continue;
		}
		retval = UA_Event_addEventToMonitoredItem(srv->m_server, &condition->m_nodeId, monitoredItem);
		Q_ASSERT(retval == UA_STATUSCODE_GOOD);
	}
	/* 3. trigger RefreshEndEvent*/
	srv->m_refreshEndEvent->setSourceNode(sourceNodeId);
	srv->m_refreshEndEvent->setSourceName(sourceDisplayName);
	srv->m_refreshEndEvent->setMessage(tr("End refresh for source %1 [%2].").arg(sourceDisplayName).arg(sourceNodeId));
	retval = UA_Event_addEventToMonitoredItem(srv->m_server, &srv->m_refreshEndEvent->m_nodeId, monitoredItem);
	Q_ASSERT(retval == UA_STATUSCODE_GOOD);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS