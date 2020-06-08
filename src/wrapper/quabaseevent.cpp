#include "quabaseevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include "quaserver_anex.h"

QUaBaseEvent::QUaBaseEvent(
	QUaServer *server
) : QUaBaseObject(server)
{
	m_sourceNodeId = UA_NODEID_NULL;
#ifdef UA_ENABLE_HISTORIZING
    // historize by default, though there might be some types for which
    // we dont want default historization like change event or condition sync
    m_historizing = true;
#endif // UA_ENABLE_HISTORIZING
	// set event type definition
	this->setEventType(this->typeDefinitionNodeId());
    this->setSeverity(0);
	// set a default parent to this is not dangling
	if (!this->parent())
	{
		this->setParent(server);
	}
}

QUaBaseEvent::~QUaBaseEvent()
{
	UA_NodeId_clear(&m_sourceNodeId);
}

QByteArray QUaBaseEvent::eventId() const
{
	return const_cast<QUaBaseEvent*>(this)->getEventId()->value().toByteArray();
}

void QUaBaseEvent::setEventId(const QByteArray& eventId)
{
    this->getEventId()->setValue(eventId);
}

QUaNodeId QUaBaseEvent::eventType() const
{
	return const_cast<QUaBaseEvent*>(this)->getEventType()->value<QUaNodeId>();
}

void QUaBaseEvent::setEventType(const QUaNodeId& eventTypeNodeId)
{
	return this->getEventType()->setValue(eventTypeNodeId);
}

QUaNodeId QUaBaseEvent::sourceNode() const
{
	return const_cast<QUaBaseEvent*>(this)->getSourceNode()->value<QUaNodeId>();
}

void QUaBaseEvent::setSourceNode(const QUaNodeId& sourceNodeId)
{
	// set cache
	UA_NodeId_clear(&m_sourceNodeId);
	m_sourceNodeId = sourceNodeId;
    // get node
    QUaNode* node = m_qUaServer->nodeById(sourceNodeId);
    QUaBaseObject* obj = qobject_cast<QUaBaseObject*>(node);
    if (node && !obj)
    {
        Q_ASSERT_X(false, "QUaBaseEvent::setSourceNode", "Source node must be a object");
        return;
    }
    if (obj)
    {
        // source node must be an event notifier
        obj->setSubscribeToEvents(true);
        //// there is supposed to be a (non-looping) event generation hierarchy
        //// TODO : implement event hierarchy according to Part 3 - 7.x
        //// for now all source nodes are directly event sources of server object directly
        //auto st = UA_Server_addReference(
        //    m_qUaServer->m_server,
        //    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER),
        //    UA_NODEID_NUMERIC(0, UA_NS0ID_HASEVENTSOURCE /*UA_NS0ID_HASNOTIFIER*/),
        //    { obj->m_nodeId, UA_STRING_NULL, 0 },
        //    true
        //);
        //Q_ASSERT(st == UA_STATUSCODE_GOOD || st == UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED);
    }
    Q_ASSERT(
          obj ||
        (!obj && sourceNodeId == UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER)) ||
        (!obj && sourceNodeId.isNull())
    );
	// set internally
	return this->getSourceNode()->setValue(sourceNodeId);
}

void QUaBaseEvent::setSourceNode(const QUaNode* sourceNode)
{
    this->setSourceNode(sourceNode ? sourceNode->nodeId()      : QUaNodeId());
    this->setSourceName(sourceNode ? sourceNode->displayName() : "");
}

QString QUaBaseEvent::sourceName() const
{
	return const_cast<QUaBaseEvent*>(this)->getSourceName()->value<QString>();
}

void QUaBaseEvent::setSourceName(const QString & strSourceName)
{
	this->getSourceName()->setValue(strSourceName);
}

QDateTime QUaBaseEvent::time() const
{
	return const_cast<QUaBaseEvent*>(this)->getTime()->value<QDateTime>().toUTC();
}

void QUaBaseEvent::setTime(const QDateTime & dateTime)
{
	this->getTime()->setValue(dateTime.toUTC());
}

QDateTime QUaBaseEvent::receiveTime() const
{
	return const_cast<QUaBaseEvent*>(this)->getReceiveTime()->value<QDateTime>().toUTC();
}

void QUaBaseEvent::setReceiveTime(const QDateTime& dateTime)
{
    this->getReceiveTime()->setValue(dateTime.toUTC());
}

// NOTE : removed because is optional and open62541 now does not add it
QTimeZone QUaBaseEvent::localTime() const
{
	return const_cast<QUaBaseEvent*>(this)->getLocalTime()->value<QTimeZone>();
}

void QUaBaseEvent::setLocalTime(const QTimeZone & localTimeZone)
{
	this->getLocalTime()->setValue(localTimeZone);
}

QUaLocalizedText QUaBaseEvent::message() const
{
	return const_cast<QUaBaseEvent*>(this)->getMessage()->value<QUaLocalizedText>();
}

void QUaBaseEvent::setMessage(const QUaLocalizedText& message)
{
	this->getMessage()->setValue(message);
}

quint16 QUaBaseEvent::severity() const
{
	return (quint16)const_cast<QUaBaseEvent*>(this)->getSeverity()->value<quint16>();
}

void QUaBaseEvent::setSeverity(const quint16 & intSeverity)
{
	this->getSeverity()->setValue(intSeverity);
}

void QUaBaseEvent::trigger()
{
    this->setEventId(QUaBaseEvent::generateEventId());
    this->triggerInternal();
}

#ifdef UA_ENABLE_HISTORIZING
bool QUaBaseEvent::historizing() const
{
	return m_historizing;
}

void QUaBaseEvent::setHistorizing(const bool& historizing)
{
    m_historizing = historizing;
}
#endif // UA_ENABLE_HISTORIZING

QUaProperty * QUaBaseEvent::getEventId() 
{
	return this->browseChild<QUaProperty>("EventId");
}

QUaProperty* QUaBaseEvent::getEventType()
{
	return this->browseChild<QUaProperty>("EventType");
}

QUaProperty* QUaBaseEvent::getSourceNode()
{
	return this->browseChild<QUaProperty>("SourceNode");
}

QUaProperty* QUaBaseEvent::getSourceName()
{
	return this->browseChild<QUaProperty>("SourceName");
}

QUaProperty* QUaBaseEvent::getTime()
{
	return this->browseChild<QUaProperty>("Time");
}

QUaProperty* QUaBaseEvent::getReceiveTime()
{
	return this->browseChild<QUaProperty>("ReceiveTime");
}

QUaProperty* QUaBaseEvent::getLocalTime()
{
	return this->browseChild<QUaProperty>("LocalTime", true);
}

QUaProperty* QUaBaseEvent::getMessage()
{
	return this->browseChild<QUaProperty>("Message");
}

QUaProperty* QUaBaseEvent::getSeverity()
{
	return this->browseChild<QUaProperty>("Severity");
}

QByteArray QUaBaseEvent::generateEventId()
{
	UA_ByteString source = QUaBaseEvent::generateEventIdInternal();
	QByteArray ret = QUaTypesConverter::uaVariantToQVariantScalar<QByteArray, UA_ByteString>(&source);
	UA_ByteString_clear(&source);
	return ret;
}

UA_ByteString QUaBaseEvent::generateEventIdInternal()
{
	UA_ByteString ret = UA_BYTESTRING_NULL;
	auto st = UA_Event_generateEventId(&ret);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st);
	return ret;
}

static const UA_NodeId objectsFolderId = { 0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_OBJECTSFOLDER} };
#define EMIT_REFS_ROOT_COUNT 4
static const UA_NodeId emitReferencesRoots[EMIT_REFS_ROOT_COUNT] ={ 
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_ORGANIZES     }},
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASCOMPONENT  }},
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASEVENTSOURCE}},
    {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_HASNOTIFIER   }} 
};

void QUaBaseEvent::triggerInternal()
{
    if (!this->shouldTrigger())
    {
        return;
    }
    // NOTE : call modified version
    auto st = QUaServer_Anex::UA_Server_triggerEvent_Modified(
        m_qUaServer->m_server,
        m_nodeId,
        m_sourceNodeId,
        nullptr
    );
    Q_ASSERT(st == UA_STATUSCODE_GOOD);
    if (st != UA_STATUSCODE_GOOD)
    {
        return;
    }
    emit this->triggered();
}

bool QUaBaseEvent::shouldTrigger() const
{
    // can trigger only if Qt parent is set
    // i.e. avoid trigger while setting props during deserialization
    return this->parent(); 
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
