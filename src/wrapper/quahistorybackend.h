#ifndef QUAHISTORYBACKEND_H
#define QUAHISTORYBACKEND_H

#include <QUaNode>

#ifdef UA_ENABLE_HISTORIZING

#include <QVector>
#include <QVariant>
#include <QDateTime>

class QUaServer;
class QUaBaseVariable;

struct QUaHistoryDataPoint
{
	QDateTime timestamp;
	QVariant  value;
	quint32   status;
};

struct QUaHistoryEventPoint
{
	QDateTime timestamp;
	QHash<QUaBrowsePath, QVariant> fields;
};

class QUaHistoryBackend
{
	friend class QUaServer;
	friend class QUaBaseVariable;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	friend class QUaServer_Anex;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
public:
	QUaHistoryBackend();

	enum class TimeMatch
	{
		ClosestFromAbove,
		ClosestFromBelow
	};

	// Type T must implement the public API below
	template<typename T>
	void setHistorizer(T& historizer);

	// write a node's data point to backend
	bool writeHistoryData(
		const QUaNodeId &nodeId, 
		const QUaHistoryDataPoint &dataPoint,
		QQueue<QUaLog> &logOut
	);
	// update an existing node's data point in backend
	bool updateHistoryData(
		const QUaNodeId &nodeId, 
		const QUaHistoryDataPoint&dataPoint,
		QQueue<QUaLog> &logOut
	);
	// remove an existing node's data points within a range
	bool removeHistoryData(
		const QUaNodeId &nodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
	); 
	// return the timestamp of the first sample available for the given node
	QDateTime firstTimestamp(
		const QUaNodeId &nodeId,
		QQueue<QUaLog> &logOut
	) const;
	// return the timestamp of the latest sample available for the given node
	QDateTime lastTimestamp(
		const QUaNodeId &nodeId,
		QQueue<QUaLog> &logOut
	) const;
	// check if given timestamp is available for the given node
	bool hasTimestamp(
		const QUaNodeId &nodeId,
		const QDateTime &timestamp,
		QQueue<QUaLog>  &logOut
	) const;
	// find a timestamp matching the criteria for the given node
	QDateTime findTimestamp(
		const QUaNodeId &nodeId,
		const QDateTime &timestamp,
		const TimeMatch &match,
		QQueue<QUaLog>  &logOut
	) const;
	// get the number for data points within a time range for the given node
	quint64 numDataPointsInRange(
		const QUaNodeId &nodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
	) const;
	// return the numPointsToRead data points for the given node from the given start time
	QVector<QUaHistoryDataPoint> readHistoryData(
		const QUaNodeId &nodeId,
		const QDateTime &timeStart,
		const quint64   &numPointsOffset,
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
		const QList<QUaBrowsePath> &columnsToRead,
		QQueue<QUaLog>  &logOut
	);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

private:

	// helpers
	static QUaHistoryDataPoint dataValueToPoint(const UA_DataValue *value);
	static UA_DataValue dataPointToValue(const QUaHistoryDataPoint *point);
	static void processServerLog(QUaServer* server, QQueue<QUaLog>& logOut);
	static QMetaType::Type QVariantToQtType(const QVariant& value);
	static void fixOutputVariantType(QVariant& value, const QMetaType::Type& metaType);

	// static and unique since implementation is instance independent
	static UA_HistoryDataBackend CreateUaBackend();
	static UA_HistoryDataBackend m_historUaBackend;

	// lambdas to capture historizer
	std::function<bool(const QUaNodeId&, const QUaHistoryDataPoint&, QQueue<QUaLog>&)> m_writeHistoryData;
	std::function<bool(const QUaNodeId&, const QUaHistoryDataPoint&, QQueue<QUaLog>&)> m_updateHistoryData;
	std::function<bool(const QUaNodeId&, const QDateTime&, const QDateTime&, QQueue<QUaLog>&)> m_removeHistoryData;
	std::function<QDateTime(const QUaNodeId&, QQueue<QUaLog>&)> m_firstTimestamp;
	std::function<QDateTime(const QUaNodeId&, QQueue<QUaLog>&)> m_lastTimestamp;
	std::function<bool(const QUaNodeId&, const QDateTime&, QQueue<QUaLog>&)> m_hasTimestamp;
	std::function<QDateTime(const QUaNodeId&, const QDateTime&, const TimeMatch&, QQueue<QUaLog>&)> m_findTimestamp;
	std::function<quint64(const QUaNodeId&, const QDateTime&, const QDateTime&, QQueue<QUaLog>&)> m_numDataPointsInRange;
	std::function<QVector<QUaHistoryDataPoint>(const QUaNodeId&, const QDateTime&, const quint64&, const quint64&, QQueue<QUaLog>&)> m_readHistoryData;

	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	std::function<bool(
		const QUaNodeId            &,
		const QList<QUaNodeId>     &,
		const QUaHistoryEventPoint &,
		QQueue<QUaLog>             &
	)> m_writeHistoryEventsOfType;
	std::function<QVector<QUaNodeId>(
		const QUaNodeId &,
		QQueue<QUaLog>  &
	)> m_eventTypesOfEmitter;
	std::function<QDateTime(
		const QUaNodeId                    &,
		const QUaNodeId                    &,
		const QDateTime                    &,
		const QUaHistoryBackend::TimeMatch &,
		QQueue<QUaLog>                     &
	)> m_findTimestampEventOfType;
	std::function<quint64(
		const QUaNodeId &,
		const QUaNodeId &,
		const QDateTime &,
		const QDateTime &,
		QQueue<QUaLog>  &
	)> m_numEventsOfTypeInRange;
	std::function<QVector<QUaHistoryEventPoint>(
		const QUaNodeId &,
		const QUaNodeId &,
		const QDateTime &,
		const quint64   &,
		const quint64   &,
		const QList<QUaBrowsePath> &,
		QQueue<QUaLog>  &
	)> m_readHistoryEventsOfType;

    static bool setEvent(
		QUaServer*                  server,
		const QUaNodeId&            eventTypeNodeId,
		const QList<QUaNodeId>&     emittersNodeIds,
		const QUaHistoryEventPoint& eventPoint
	);
    static void readEvent(
        UA_Server*                    server,
        void*                         hdbContext,
        const UA_NodeId*              sessionId,
        void*                         sessionContext,
        const UA_RequestHeader*       requestHeader,
        const UA_ReadEventDetails*    historyReadDetails,
        UA_TimestampsToReturn         timestampsToReturn,
        UA_Boolean                    releaseContinuationPoints,
        size_t                        nodesToReadSize,
        const UA_HistoryReadValueId*  nodesToRead,
        UA_HistoryReadResponse*       response,
        UA_HistoryEvent* const* const historyData
    );
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

};

template<typename T>
inline void QUaHistoryBackend::setHistorizer(T& historizer)
{
	// writeHistoryData
	m_writeHistoryData = [&historizer](
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint &dataPoint,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.writeHistoryData(
				nodeId,
				dataPoint,
				logOut
			);
	};
	// updateHistoryData
	m_updateHistoryData = [&historizer](
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint &dataPoint,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.updateHistoryData(
				nodeId,
				dataPoint,
				logOut
			);
	};
	// removeHistoryData
	m_removeHistoryData = [&historizer](
		const QUaNodeId &nodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.removeHistoryData(
				nodeId,
				timeStart,
				timeEnd,
				logOut
			);
	};
	// firstTimestamp
	m_firstTimestamp = [&historizer](
		const QUaNodeId &nodeId,
		QQueue<QUaLog> &logOut
		) -> QDateTime {
			return historizer.firstTimestamp(
				nodeId,
				logOut
			);
	};
	// lastTimestamp
	m_lastTimestamp = [&historizer](
		const QUaNodeId &nodeId,
		QQueue<QUaLog> &logOut
		) -> QDateTime {
			return historizer.lastTimestamp(
				nodeId,
				logOut
			);
	};
	// hasTimestamp
	m_hasTimestamp = [&historizer](
		const QUaNodeId &nodeId,
		const QDateTime &timestamp,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.hasTimestamp(
				nodeId,
				timestamp,
				logOut
			);
	};
	// findTimestamp
	m_findTimestamp = [&historizer](
		const QUaNodeId &nodeId,
		const QDateTime &timestamp,
		const TimeMatch &match,
		QQueue<QUaLog>  &logOut
		) -> QDateTime {
			return historizer.findTimestamp(
				nodeId,
				timestamp,
				match,
				logOut
			);
	};
	// numDataPointsInRange
	m_numDataPointsInRange = [&historizer](
		const QUaNodeId &nodeId, 
		const QDateTime &timeStart, 
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
		) -> quint64 {
			return historizer.numDataPointsInRange(
				nodeId,
				timeStart,
				timeEnd,
				logOut
			);
	};
	// readHistoryData
	m_readHistoryData = [&historizer](
		const QUaNodeId &nodeId, 
		const QDateTime &timeStart, 
		const quint64   &numPointsOffset,
		const quint64   &numPointsToRead,
		QQueue<QUaLog>  &logOut
		) -> QVector<QUaHistoryDataPoint>{
			return historizer.readHistoryData(
				nodeId,
				timeStart,
				numPointsOffset,
				numPointsToRead,
				logOut
			);
	};

	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// writeHistoryEventsOfType
	m_writeHistoryEventsOfType = [&historizer](
		const QUaNodeId            &eventTypeNodeId,
		const QList<QUaNodeId>     &emittersNodeIds,
		const QUaHistoryEventPoint &eventPoint,
		QQueue<QUaLog>             &logOut
	) -> bool{
			return historizer.writeHistoryEventsOfType(
				eventTypeNodeId,
				emittersNodeIds,
				eventPoint,
				logOut
			);
	};
	// findTimestampEventOfType
	m_findTimestampEventOfType = [&historizer](
		const QUaNodeId                    &emitterNodeId,
		const QUaNodeId                    &eventTypeNodeId,
		const QDateTime                    &timestamp,
		const QUaHistoryBackend::TimeMatch &match,
		QQueue<QUaLog>                     &logOut
	) ->QDateTime{
			return historizer.findTimestampEventOfType(
				emitterNodeId,
				eventTypeNodeId,
				timestamp,
				match,
				logOut
			);
	};
	// eventTypesOfEmitter
	m_eventTypesOfEmitter = [&historizer](
		const QUaNodeId &emitterNodeId,
		QQueue<QUaLog>  &logOut
	) -> QVector<QUaNodeId> {
			return historizer.eventTypesOfEmitter(
				emitterNodeId,
				logOut
			);
	};
	// numEventsOfTypeInRange
	m_numEventsOfTypeInRange = [&historizer](
		const QUaNodeId &emitterNodeId,
		const QUaNodeId &eventTypeNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
		) -> quint64
	{
		return historizer.numEventsOfTypeInRange(
			emitterNodeId,
			eventTypeNodeId,
			timeStart,
			timeEnd,
			logOut
		);
	};
	// readHistoryEventsOfType
	m_readHistoryEventsOfType = [&historizer](
		const QUaNodeId &emitterNodeId,
		const QUaNodeId &eventTypeNodeId,
		const QDateTime &timeStart,
		const quint64   &numPointsOffset,
		const quint64   &numPointsToRead,
		const QList<QUaBrowsePath> &columnsToRead,
		QQueue<QUaLog>  &logOut
	) -> QVector<QUaHistoryEventPoint>
	{
		return historizer.readHistoryEventsOfType(
			emitterNodeId,
			eventTypeNodeId,
			timeStart,
			numPointsOffset,
			numPointsToRead,
			columnsToRead,
			logOut
		);
	};
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

}

#endif // UA_ENABLE_HISTORIZING

#endif // QUAHISTORYBACKEND_H
