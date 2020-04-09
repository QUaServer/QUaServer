#ifndef QUABASEMODELCHANGEEVENT_H
#define QUABASEMODELCHANGEEVENT_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part 5 - 6.4.31

Concept introduced in Part 3 - 9.32.5
BaseModelChangeEvents are Events of the BaseModelChangeEventType. The
BaseModelChangeEventType is the base type for ModelChangeEvents and does not contain
information about the changes but only indicates that changes occurred. Therefore the Client
shall assume that any or all of the Nodes may have changed.

This EventType inherits all Properties of the BaseEventType. Their semantic is defined in 6.4.2.
There are no additional Properties defined for this EventType. The SourceNode Property for
Events of this type shall be the Node of the View that gives the context of the changes. If the
whole AddressSpace is the context, the SourceNode Property is set to the NodeId of the Server
Object. The SourceName for Events of this type shall be the String part of the BrowseName of
the View; for the whole AddressSpace it shall be “Server”.

*/

class QUaBaseModelChangeEvent : public QUaBaseEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaBaseModelChangeEvent(
		QUaServer *server
	);

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUABASEMODELCHANGEEVENT_H