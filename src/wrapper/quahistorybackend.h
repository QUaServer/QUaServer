#ifndef QUAHISTORYBACKEND_H
#define QUAHISTORYBACKEND_H

#include <QVector>
#include <QVariant>
#include <QDateTime>

#include <QUaNode>

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

	// write a node's data point to backend
	bool writeHistoryData(
		const QString   &strNodeId, 
		const DataPoint &dataPoint,
		QQueue<QUaLog>  &logOut
	);
	// update an existing node's data point in backend
	bool updateHistoryData(
		const QString   &strNodeId, 
		const DataPoint &dataPoint,
		QQueue<QUaLog>  &logOut
	);
	// remove an existing node's data points within a range
	bool removeHistoryData(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
	); 
	// return the timestamp of the first sample available for the given node
	QDateTime firstTimestamp(
		const QString  &strNodeId,
		QQueue<QUaLog> &logOut
	) const;
	// return the timestamp of the latest sample available for the given node
	QDateTime lastTimestamp(
		const QString  &strNodeId,
		QQueue<QUaLog> &logOut
	) const;
	// check if given timestamp is available for the given node
	bool hasTimestamp(
		const QString   &strNodeId,
		const QDateTime &timestamp,
		QQueue<QUaLog>  &logOut
	) const;
	// find a timestamp matching the criteria for the given node
	QDateTime findTimestamp(
		const QString   &strNodeId,
		const QDateTime &timestamp,
		const TimeMatch &match,
		QQueue<QUaLog>  &logOut
	) const;
	// get the number for data points within a time range for the given node
	quint64 numDataPointsInRange(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
	) const;
	// return the numPointsToRead data points for the given node,
	//             starting from numPointsAlreadyRead within the given time range.
	QVector<DataPoint> readHistoryData(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		const quint64   &numPointsAlreadyRead,
		const quint64   &numPointsToRead,
		QQueue<QUaLog>  &logOut
	) const;

private:

	// helpers
	static DataPoint dataValueToPoint(const UA_DataValue *value);
	static UA_DataValue dataPointToValue(const DataPoint *point);
	static void processServerLog(QUaServer* server, QQueue<QUaLog>& logOut);

	// static and unique since implementation is instance independent
	static UA_HistoryDataBackend CreateUaBackend();
	static UA_HistoryDataBackend m_historUaBackend;

	// lambdas to capture historizer
	std::function<bool(const QString&, const DataPoint&, QQueue<QUaLog>&)> m_writeHistoryData;
	std::function<bool(const QString&, const DataPoint&, QQueue<QUaLog>&)> m_updateHistoryData;
	std::function<bool(const QString&, const QDateTime&, const QDateTime&, QQueue<QUaLog>&)> m_removeHistoryData;
	std::function<QDateTime(const QString&, QQueue<QUaLog>&)> m_firstTimestamp;
	std::function<QDateTime(const QString&, QQueue<QUaLog>&)> m_lastTimestamp;
	std::function<bool(const QString&, const QDateTime&, QQueue<QUaLog>&)> m_hasTimestamp;
	std::function<QDateTime(const QString&, const QDateTime&, const TimeMatch&, QQueue<QUaLog>&)> m_findTimestamp;
	std::function<quint64(const QString&, const QDateTime&, const QDateTime&, QQueue<QUaLog>&)> m_numDataPointsInRange;
	std::function<QVector<QUaHistoryBackend::DataPoint>(const QString&, const QDateTime&, const QDateTime&, const quint64&, const quint64&, QQueue<QUaLog>&)> m_readHistoryData;


};

template<typename T>
inline void QUaHistoryBackend::setHistorizer(T& historizer)
{
	// writeHistoryData
	m_writeHistoryData = [&historizer](
		const QString   &strNodeId,
		const DataPoint &dataPoint,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.writeHistoryData(
				strNodeId,
				dataPoint,
				logOut
			);
	};
	// updateHistoryData
	m_updateHistoryData = [&historizer](
		const QString   &strNodeId,
		const DataPoint &dataPoint,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.updateHistoryData(
				strNodeId,
				dataPoint,
				logOut
			);
	};
	// removeHistoryData
	m_removeHistoryData = [&historizer](
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.removeHistoryData(
				strNodeId,
				timeStart,
				timeEnd,
				logOut
			);
	};
	// firstTimestamp
	m_firstTimestamp = [&historizer](
		const QString  &strNodeId,
		QQueue<QUaLog> &logOut
		) -> QDateTime {
			return historizer.firstTimestamp(
				strNodeId,
				logOut
			);
	};
	// lastTimestamp
	m_lastTimestamp = [&historizer](
		const QString  &strNodeId,
		QQueue<QUaLog> &logOut
		) -> QDateTime {
			return historizer.lastTimestamp(
				strNodeId,
				logOut
			);
	};
	// hasTimestamp
	m_hasTimestamp = [&historizer](
		const QString   &strNodeId,
		const QDateTime &timestamp,
		QQueue<QUaLog>  &logOut
		) -> bool {
			return historizer.hasTimestamp(
				strNodeId,
				timestamp,
				logOut
			);
	};
	// findTimestamp
	m_findTimestamp = [&historizer](
		const QString   &strNodeId,
		const QDateTime &timestamp,
		const TimeMatch &match,
		QQueue<QUaLog>  &logOut
		) -> QDateTime {
			return historizer.findTimestamp(
				strNodeId,
				timestamp,
				match,
				logOut
			);
	};
	// numDataPointsInRange
	m_numDataPointsInRange = [&historizer](
		const QString   &strNodeId, 
		const QDateTime &timeStart, 
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
		) -> quint64 {
			return historizer.numDataPointsInRange(
				strNodeId,
				timeStart,
				timeEnd,
				logOut
			);
	};
	// readHistoryData
	m_readHistoryData = [&historizer](
		const QString   &strNodeId, 
		const QDateTime &timeStart, 
		const QDateTime &timeEnd, 
		const quint64   &numPointsAlreadyRead, 
		const quint64   &numPointsToRead,
		QQueue<QUaLog>  &logOut
		) -> QVector<QUaHistoryBackend::DataPoint>{
			return historizer.readHistoryData(
				strNodeId,
				timeStart,
				timeEnd,
				numPointsAlreadyRead,
				numPointsToRead,
				logOut
			);
	};
}


#endif // QUAHISTORYBACKEND_H