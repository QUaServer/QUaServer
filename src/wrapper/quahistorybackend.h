#ifndef QUAHISTORYBACKEND_H
#define QUAHISTORYBACKEND_H

#include <QVector>
#include <QVariant>
#include <QDateTime>

class QUaServer;
class QUaBaseVariable;

class QUaHistoryBackend
{
	friend class QUaServer;
	friend class QUaBaseVariable;
public:
	QUaHistoryBackend();

	struct DataPoint
	{
		QDateTime timestamp;
		QVariant  value;
		quint32   status;
	};

	enum class TimeMatch
	{
		ClosestFromAbove,
		ClosestFromBelow
	};

	// Type T must implement the public API below
	template<typename T>
	void setHistorizer(T& historizer);

	// Mandatory : write a node's data point to backend
	bool writeHistoryData(
		const QString   &strNodeId, 
		const DataPoint &dataPoint
	);
	// Mandatory : return the timestamp of the first sample available for the given node
	QDateTime firstTimestamp(
		const QString &strNodeId
	) const;
	// Mandatory : return the timestamp of the latest sample available for the given node
	QDateTime lastTimestamp(
		const QString &strNodeId
	) const;
	// Mandatory : check if given timestamp is available for the given node
	bool hasTimestamp(
		const QString   &strNodeId,
		const QDateTime &timestamp
	) const;
	// Mandatory : find a timestamp matching the criteria for the given node
	QDateTime findTimestamp(
		const QString   &strNodeId,
		const QDateTime &timestamp,
		const TimeMatch &match
	) const;
	// Mandatory : get the number for data points within a time range for the given node
	quint64 numDataPointsInRange(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd
	) const;
	// Mandatory : return the numPointsToRead data points for the given node,
	//             starting from numPointsAlreadyRead within the given time range.
	QVector<DataPoint> readHistoryData(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		const quint64   &numPointsAlreadyRead,
		const quint64   &numPointsToRead
	) const;

private:

	// helpers
	static DataPoint dataValueToPoint(const UA_DataValue *value);
	static UA_DataValue dataPointToValue(const DataPoint *point);

	// static and unique since implementation is instance independent
	static UA_HistoryDataBackend CreateUaBackend();
	static UA_HistoryDataBackend m_historUaBackend;

	// lambdas to capture historizer
	std::function<bool(const QString&, const DataPoint&)> m_writeHistoryData;
	std::function<QDateTime(const QString&)>              m_firstTimestamp;
	std::function<QDateTime(const QString&)>              m_lastTimestamp;
	std::function<bool(const QString&, const QDateTime&)> m_hasTimestamp;
	std::function<QDateTime(const QString&, const QDateTime&, const TimeMatch&)> m_findTimestamp;
	std::function<quint64(const QString&, const QDateTime&, const QDateTime&)>   m_numDataPointsInRange;
	std::function<QVector<QUaHistoryBackend::DataPoint>(const QString&, const QDateTime&, const QDateTime&, const quint64&, const quint64&)> m_readHistoryData;


};

template<typename T>
inline void QUaHistoryBackend::setHistorizer(T& historizer)
{
	// writeHistoryData
	m_writeHistoryData = [&historizer](
		const QString   &strNodeId,
		const DataPoint &dataPoint
		) -> bool {
			return historizer.writeHistoryData(
				strNodeId,
				dataPoint
			);
	};
	// firstTimestamp
	m_firstTimestamp = [&historizer](
		const QString &strNodeId
		) -> QDateTime {
			return historizer.firstTimestamp(
				strNodeId
			);
	};
	// lastTimestamp
	m_lastTimestamp = [&historizer](
		const QString &strNodeId
		) -> QDateTime {
			return historizer.lastTimestamp(
				strNodeId
			);
	};
	// hasTimestamp
	m_hasTimestamp = [&historizer](
		const QString   &strNodeId,
		const QDateTime &timestamp
		) -> bool {
			return historizer.hasTimestamp(
				strNodeId,
				timestamp
			);
	};
	// findTimestamp
	m_findTimestamp = [&historizer](
		const QString   &strNodeId,
		const QDateTime &timestamp,
		const TimeMatch &match
		) -> QDateTime {
			return historizer.findTimestamp(
				strNodeId,
				timestamp,
				match
			);
	};
	// numDataPointsInRange
	m_numDataPointsInRange = [&historizer](
		const QString   &strNodeId, 
		const QDateTime &timeStart, 
		const QDateTime &timeEnd
		) -> quint64 {
			return historizer.numDataPointsInRange(
				strNodeId,
				timeStart,
				timeEnd
			);
	};
	// readHistoryData
	m_readHistoryData = [&historizer](
		const QString   &strNodeId, 
		const QDateTime &timeStart, 
		const QDateTime &timeEnd, 
		const quint64   &numPointsAlreadyRead, 
		const quint64   &numPointsToRead
		) -> QVector<QUaHistoryBackend::DataPoint>{
			return historizer.readHistoryData(
				strNodeId,
				timeStart,
				timeEnd,
				numPointsAlreadyRead,
				numPointsToRead
			);
	};
}


#endif // QUAHISTORYBACKEND_H