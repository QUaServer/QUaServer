#ifndef QUAGENERALMODELCHANGEEVENT_H
#define QUAGENERALMODELCHANGEEVENT_H

#include <QUaBaseEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

/*
Part 3 - 8.20.6 GeneralModelChangeEventType (pag 55 or 67 in pdf)
The GeneralModelChangeEventType is a subtype of the BaseModelChangeEventType. 
It contains information about the Node that was changed and the action that occurred the ModelChangeEvent (e.g. add a Node, delete a Node, etc.). 
If the affected Node is a Variable or Object, the TypeDefinitionNode is also present.
To allow Event compression, a GeneralModelChangeEvent contains an array of this structure.


*/

typedef QVector<QUaChangeStructureDataType> QUaChangesList;

class QUaGeneralModelChangeEvent : public QUaBaseEvent
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaGeneralModelChangeEvent(QUaServer *server);


	QUaChangesList changes() const;
	void           setChanges(const QUaChangesList &listVerbs);

	// This is used in QUaServer::createEvent and QUaNode constructor to add event properties defined in standard as children QObjects
	static const QStringList DefaultProperties;

private:
	// ChangeStructureDataType (PArt 5 - 11.14) : UA_ModelChangeStructureDataType
	QUaProperty * getChanges() const;

};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUAGENERALMODELCHANGEEVENT_H