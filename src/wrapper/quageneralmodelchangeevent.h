#ifndef QUAGENERALMODELCHANGEEVENT_H
#define QUAGENERALMODELCHANGEEVENT_H

#include <QUaBaseModelChangeEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part 5 - 6.4.32

Concept introduced in Part 3 - 9.32.6
GeneralModelChangeEvents are Events of the GeneralModelChangeEventType. The
GeneralModelChangeEventType is a subtype of the BaseModelChangeEventType. It contains
information about the Node that was changed and the action that occurred to cause the
ModelChangeEvent (e.g. add a Node, delete a Node, etc.). If the affected Node is a Variable or
Object, then the TypeDefinitionNode is also present.
To allow Event compression, a GeneralModelChangeEvent contains an array of changes.

This EventType inherits all Properties of the BaseModelChangeEventType. Their semantic is
defined in 6.4.31.
The additional Property defined for this EventType reflects the changes that issued the
ModelChangeEvent. It shall contain at least one entry in its array. Its structure is defined in
Part 5 - 12.16

*/

class QUaGeneralModelChangeEvent : public QUaBaseModelChangeEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaGeneralModelChangeEvent(
		QUaServer *server
	);


	QUaChangesList changes() const;
	void           setChanges(const QUaChangesList &listVerbs);

private:
	// ChangeStructureDataType (PArt 5 - 11.14) : UA_ModelChangeStructureDataType
	QUaProperty * getChanges() const;

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUAGENERALMODELCHANGEEVENT_H