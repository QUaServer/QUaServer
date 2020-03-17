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

private:
	bool writeHistoryData(
		const QString   &strNodeId, 
		const DataPoint &dataPoint
	);
	QDateTime firstTimestamp(
		const QString& strNodeId
	) const;
	QDateTime lastTimestamp(
		const QString& strNodeId
	) const;
	bool hasTimestamp(
		const QString& strNodeId,
		const QDateTime& timestamp
	) const;
	QDateTime findTimestamp(
		const QString   &strNodeId,
		const QDateTime &timestamp,
		const TimeMatch &match
	) const;
	quint64 numDataPointsInRange(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd
	) const;
	QVector<DataPoint> readHistoryData(
		const QString   &strNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		const quint64   &numPointsAlreadyRead,
		const quint64   &numPointsToRead,
		const bool      &startFromEnd
	) const;

	// helpers
	static DataPoint dataValueToPoint(const UA_DataValue *value);
	static UA_DataValue dataPointToValue(const DataPoint *point);

	// static and unique since implementation is instance independent
	static UA_HistoryDataBackend CreateUaBackend();
	static UA_HistoryDataBackend m_historUaBackend;
};

#endif // QUAHISTORYBACKEND_H