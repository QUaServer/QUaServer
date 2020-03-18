#ifndef QUAINMEMORYHISTORIZER_H
#define QUAINMEMORYHISTORIZER_H

#include <QUaHistoryBackend>

class QUaInMemoryHistoryBackend
{
public:

	// Mandatory : write data point to backend
	bool writeHistoryData(
		const QString &strNodeId,
		const QUaHistoryBackend::DataPoint &dataPoint
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
		const QUaHistoryBackend::TimeMatch& match
	) const;
	// Mandatory : get the number for data points within a time range for the given node
	quint64 numDataPointsInRange(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd
	) const;
	// Mandatory : return the numPointsToRead data points for the given node,
	//             starting from numPointsAlreadyRead within the given time range.
	//             if startFromEnd is true, return numPointsToRead starting with the
	//             most recent ones
	QVector<QUaHistoryBackend::DataPoint> readHistoryData(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		const quint64   &numPointsAlreadyRead,
		const quint64   &numPointsToRead,
		const bool      &startFromEnd
	) const;

private:
	struct DataPoint
	{
		QVariant  value;
		quint32   status;
	};
	// NOTE : use a map to store the data points of a single node, ordered by time
	typedef QMap<QDateTime, DataPoint> DataPointTable;
	QHash<QString, DataPointTable> m_database;
};

#endif // QUAINMEMORYHISTORIZER_H
