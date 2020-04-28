#ifndef QUAINMEMORYHISTORIZER_H
#define QUAINMEMORYHISTORIZER_H

#include <QUaHistoryBackend>

#ifdef UA_ENABLE_HISTORIZING

class QUaInMemoryHistorizer
{
public:

	// required API for QUaServer::setHistorizer
	// write data point to backend, return true on success
	bool writeHistoryData(
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// update an existing node's data point in backend, return true on success
	bool updateHistoryData(
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// remove an existing node's data points within a range, return true on success
	bool removeHistoryData(
		const QUaNodeId &nodeId,
		const QDateTime& timeStart,
		const QDateTime& timeEnd,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the timestamp of the first sample available for the given node
	QDateTime firstTimestamp(
		const QUaNodeId &nodeId,
		QQueue<QUaLog>& logOut
	) const;
	// required API for QUaServer::setHistorizer
	// return the timestamp of the latest sample available for the given node
	QDateTime lastTimestamp(
		const QUaNodeId &nodeId,
		QQueue<QUaLog>& logOut
	) const;
	// required API for QUaServer::setHistorizer
	// return true if given timestamp is available for the given node
	bool hasTimestamp(
		const QUaNodeId &nodeId,
		const QDateTime& timestamp,
		QQueue<QUaLog>& logOut
	) const;
	// required API for QUaServer::setHistorizer
	// return a timestamp matching the criteria for the given node
	QDateTime findTimestamp(
		const QUaNodeId &nodeId,
		const QDateTime& timestamp,
		const QUaHistoryBackend::TimeMatch& match,
		QQueue<QUaLog>& logOut
	) const;
	// required API for QUaServer::setHistorizer
	// return the number for data points within a time range for the given node
	quint64 numDataPointsInRange(
		const QUaNodeId &nodeId,
		const QDateTime& timeStart,
		const QDateTime& timeEnd,
		QQueue<QUaLog>& logOut
	) const;
	// required API for QUaServer::setHistorizer
	// return the numPointsToRead data points for the given node from the given start time
	QVector<QUaHistoryDataPoint> readHistoryData(
		const QUaNodeId & nodeId,
		const QDateTime &timeStart,
		const quint64& numPointsOffset,
		const quint64   &numPointsToRead,
		QQueue<QUaLog>  &logOut
	) const;

	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// write a event's data to backend
	bool writeHistoryEventsOfType(
		const QUaNodeId            &eventTypeNodeId,
		const QList<QUaNodeId>     &emittersNodeIds,
		const QUaHistoryEventPoint &eventPoint,
		QQueue<QUaLog>             &logOut
	);
	// get event types (node ids) for which there are events stored for the
	// given emitter
	QVector<QUaNodeId> eventTypesOfEmitter(
		const QUaNodeId &emitterNodeId,
		QQueue<QUaLog>  &logOut
	);
	// find a timestamp matching the criteria for the emitter and event type
	QDateTime findTimestampEventOfType(
		const QUaNodeId                    &emitterNodeId,
		const QUaNodeId                    &eventTypeNodeId,
		const QDateTime                    &timestamp,
		const QUaHistoryBackend::TimeMatch &match,
		QQueue<QUaLog>                     &logOut
	);
	// get the number for events within a time range for the given emitter and event type
	quint64 numEventsOfTypeInRange(
		const QUaNodeId &emitterNodeId,
		const QUaNodeId &eventTypeNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
	);
	// return the numPointsToRead events for the given emitter and event type,
	// starting from the numPointsOffset offset after given start time (pagination)
	QVector<QUaHistoryEventPoint> readHistoryEventsOfType(
		const QUaNodeId &emitterNodeId,
		const QUaNodeId &eventTypeNodeId,
		const QDateTime &timeStart,
		const quint64   &numPointsOffset,
		const quint64   &numPointsToRead,
		QQueue<QUaLog>  &logOut
	);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

private:
	struct DataPoint
	{
		QVariant  value;
		quint32   status;
	};
	// NOTE : use a map to store the data points of a single node, ordered by time
	typedef QMap<QDateTime, DataPoint> DataPointTable;
	QHash<QUaNodeId, DataPointTable> m_database;

	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	typedef QHash<uint        /*EventKey*/, QUaHistoryEventPoint> QUaEventTypeTable;
	typedef QHash<QUaNodeId /*TypeNodeId*/, QUaEventTypeTable   > QUaEventTypeDatabase;
	QUaEventTypeDatabase m_eventTypeDatabase;

	typedef QMultiMap <QDateTime, uint     /*EventKey*/> QUaEventEmitterTable;
	typedef QHash     <QUaNodeId /*TypeNodeId*/, QUaEventEmitterTable > QUaEventTypeIndex;
	typedef QHash     <QUaNodeId  /*EmitterId*/, QUaEventTypeIndex    > QUaEventEmitterDatabase;
	QUaEventEmitterDatabase m_eventEmitterDatabase;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

};

#endif // UA_ENABLE_HISTORIZING

#endif // QUAINMEMORYHISTORIZER_H
