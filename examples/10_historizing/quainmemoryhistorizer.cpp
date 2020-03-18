#include "quainmemoryhistorizer.h"

bool QUaInMemoryHistoryBackend::writeHistoryData(
	const QString &strNodeId, 
	const QUaHistoryBackend::DataPoint &dataPoint)
{
	m_database[strNodeId][dataPoint.timestamp] = {
		dataPoint.value,
		dataPoint.status
	};
	return true;
}

QDateTime QUaInMemoryHistoryBackend::firstTimestamp(
	const QString &strNodeId) const
{
	if (!m_database.contains(strNodeId))
	{
		return QDateTime();
	}
	return m_database[strNodeId].firstKey();
}

QDateTime QUaInMemoryHistoryBackend::lastTimestamp(
	const QString &strNodeId) const
{
	if (!m_database.contains(strNodeId))
	{
		return QDateTime();
	}
	return m_database[strNodeId].lastKey();
}

bool QUaInMemoryHistoryBackend::hasTimestamp(
	const QString   &strNodeId, 
	const QDateTime &timestamp) const
{
	if (!m_database.contains(strNodeId))
	{
		return false;
	}
	return m_database[strNodeId].contains(timestamp);
}

QDateTime QUaInMemoryHistoryBackend::findTimestamp(
	const QString   &strNodeId, 
	const QDateTime &timestamp, 
	const QUaHistoryBackend::TimeMatch &match) const
{
	if (!m_database.contains(strNodeId))
	{
		return QDateTime();
	}
	QDateTime time;
	auto& table = m_database[strNodeId];
	switch (match)
	{
		case QUaHistoryBackend::TimeMatch::ClosestFromAbove:
		{
			if (table.contains(timestamp) &&
				table.constFind(timestamp) + 1 != table.end())
			{
				// return next key if available
				auto iter = table.find(timestamp) + 1;
				time = iter.key();
			}
			else
			{
				// return closest key from above or last one if out of range
				auto iter = std::upper_bound(table.keyBegin(), table.keyEnd(), timestamp);
				time = iter == table.keyEnd() ? table.lastKey() : *iter;
			}
		}
		break;
		case QUaHistoryBackend::TimeMatch::ClosestFromBelow:
		{
			if (table.contains(timestamp) &&
				table.constFind(timestamp) != table.begin())
			{
				// return previous key if available
				auto iter = table.find(timestamp) - 1;
				time = iter.key();
			}
			else
			{
				// return closest key from below or last one if out of range
				auto iter = std::lower_bound(table.keyBegin(), table.keyEnd(), timestamp);
				time = iter == table.keyEnd() ? table.lastKey() : *iter;
			}
		}
		break;
		default:
		break;
	}
	return time;
}

quint64 QUaInMemoryHistoryBackend::numDataPointsInRange(
	const QString   &strNodeId, 
	const QDateTime &timeStart, 
	const QDateTime &timeEnd) const
{
	if (!m_database.contains(strNodeId))
	{
		return 0;
	}
	auto& table = m_database[strNodeId];
	Q_ASSERT(table.contains(timeStart));
	Q_ASSERT(table.contains(timeEnd) || !timeEnd.isValid());
	return static_cast<quint64>(std::distance(
		table.find(timeStart), 
		timeEnd.isValid() ? table.find(timeEnd) + 1 : table.end()
	));
}

QVector<QUaHistoryBackend::DataPoint> QUaInMemoryHistoryBackend::readHistoryData(
	const QString   &strNodeId, 
	const QDateTime &timeStart, 
	const QDateTime &timeEnd, 
	const quint64   &numPointsAlreadyRead, 
	const quint64   &numPointsToRead, 
	const bool      &startFromEnd) const
{
	auto points = QVector<QUaHistoryBackend::DataPoint>();
	if (!m_database.contains(strNodeId))
	{
		return points;
	}
	auto& table = m_database[strNodeId];
	Q_ASSERT(table.contains(timeStart));
	Q_ASSERT(table.contains(timeEnd) || !timeEnd.isValid());
	// get total range to copy
	auto iterIni = table.find(timeStart);
	auto iterEnd = timeEnd.isValid() ? table.find(timeEnd) + 1 : table.end();
	// resize return value accordingly
	points.resize(numPointsToRead);
	// range of data points copy depend on direction
	decltype(iterIni) iterCopy;
	if (!startFromEnd)
	{
		iterCopy = iterIni + numPointsAlreadyRead;
		
	}
	else
	{
		iterCopy = iterEnd - 1 - numPointsAlreadyRead - numPointsToRead;
	}
	// copy data points
	std::generate(points.begin(), points.end(),
	[&iterCopy]() {
		QUaHistoryBackend::DataPoint retVal = {
			iterCopy.key(),
			iterCopy.value().value,
			iterCopy.value().status
		};
		iterCopy++;
		return retVal;
	});
	return points;
}
