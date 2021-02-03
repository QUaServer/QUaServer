#ifndef QUAGENERALMODELCHANGEEVENT_H
#define QUAGENERALMODELCHANGEEVENT_H

#include <QUaBaseModelChangeEvent>

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

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
	QUaProperty * getChanges();
	QUaProperty * m_changes;
};

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // QUAGENERALMODELCHANGEEVENT_H