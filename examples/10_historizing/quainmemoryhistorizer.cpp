#include "quainmemoryhistorizer.h"

bool QUaInMemoryHistorizer::writeHistoryData(
	const QString &strNodeId, 
	const QUaHistoryBackend::DataPoint &dataPoint,
	QQueue<QUaLog> &logOut)
{
	Q_UNUSED(logOut);
	m_database[strNodeId][dataPoint.timestamp] = {
		dataPoint.value,
		dataPoint.status
	};
	return true;
}

bool QUaInMemoryHistorizer::updateHistoryData(
	const QString &strNodeId, 
	const QUaHistoryBackend::DataPoint &dataPoint,
	QQueue<QUaLog> &logOut)
{
	Q_UNUSED(logOut);
	Q_ASSERT(
		m_database.contains(strNodeId) && 
		m_database[strNodeId].contains(dataPoint.timestamp)
	);
	return this->writeHistoryData(strNodeId, dataPoint, logOut);
}

bool QUaInMemoryHistorizer::removeHistoryData(
	const QString   &strNodeId, 
	const QDateTime &timeStart, 
	const QDateTime &timeEnd,
	QQueue<QUaLog>  &logOut)
{
	Q_ASSERT(timeStart <= timeEnd);
	if (!m_database.contains(strNodeId))
	{
		logOut << QUaLog({
			QObject::tr("Error removing history data. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	auto& table = m_database[strNodeId];
	Q_ASSERT(table.contains(timeStart));
	Q_ASSERT(table.contains(timeEnd) || !timeEnd.isValid());
	// get total range to remove
	auto iterIni = table.find(timeStart);
	auto iterEnd = timeEnd.isValid() ? table.find(timeEnd) + 1 : table.end();
	// remove range
	for (auto it = iterIni; it != iterEnd; it++)
	{
		table.erase(it);
	}
	return true;
}

QDateTime QUaInMemoryHistorizer::firstTimestamp(
	const QString  &strNodeId,
	QQueue<QUaLog> &logOut) const
{
	if (!m_database.contains(strNodeId))
	{
		logOut << QUaLog({
			QObject::tr("Error finding first history timestamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return QDateTime();
	}
	return m_database[strNodeId].firstKey();
}

QDateTime QUaInMemoryHistorizer::lastTimestamp(
	const QString  &strNodeId,
	QQueue<QUaLog> &logOut) const
{
	if (!m_database.contains(strNodeId))
	{
		logOut << QUaLog({
			QObject::tr("Error finding most recent history timstamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return QDateTime();
	}
	return m_database[strNodeId].lastKey();
}

bool QUaInMemoryHistorizer::hasTimestamp(
	const QString   &strNodeId, 
	const QDateTime &timestamp,
	QQueue<QUaLog>  &logOut) const
{
	if (!m_database.contains(strNodeId))
	{
		logOut << QUaLog({
			QObject::tr("Error finding history timestamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	return m_database[strNodeId].contains(timestamp);
}

QDateTime QUaInMemoryHistorizer::findTimestamp(
	const QString   &strNodeId, 
	const QDateTime &timestamp, 
	const QUaHistoryBackend::TimeMatch &match,
	QQueue<QUaLog>  &logOut) const
{
	if (!m_database.contains(strNodeId))
	{
		logOut << QUaLog({
			QObject::tr("Error finding history timestamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
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
				Q_ASSERT(timestamp <= *table.keyBegin());
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

quint64 QUaInMemoryHistorizer::numDataPointsInRange(
	const QString   &strNodeId, 
	const QDateTime &timeStart, 
	const QDateTime &timeEnd,
	QQueue<QUaLog>  &logOut) const
{
	if (!m_database.contains(strNodeId))
	{
		logOut << QUaLog({
			QObject::tr("Error finding history points. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
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

QVector<QUaHistoryBackend::DataPoint> QUaInMemoryHistorizer::readHistoryData(
	const QString   &strNodeId, 
	const QDateTime &timeStart, 
	const QDateTime &timeEnd, 
	const quint64   &numPointsAlreadyRead, 
	const quint64   &numPointsToRead,
	QQueue<QUaLog>  &logOut) const
{
	auto points = QVector<QUaHistoryBackend::DataPoint>();
	if (!m_database.contains(strNodeId))
	{
		logOut << QUaLog({
			QObject::tr("Error reading history data. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return points;
	}
	auto& table = m_database[strNodeId];
	// NOTE : timeStart can be == timeEnd
	Q_ASSERT(timeStart <= timeEnd);
	Q_ASSERT(table.contains(timeStart));
	Q_ASSERT(table.contains(timeEnd) || !timeEnd.isValid());
	// get total range to read
	auto iterIni = table.find(timeStart);
	auto iterEnd = timeEnd.isValid() ? table.find(timeEnd) + 1 : table.end();
	// resize return value accordingly
	points.resize(numPointsToRead);
	// read from start point
	auto iterCopy = iterIni + numPointsAlreadyRead;
	Q_ASSERT(std::distance(iterCopy, iterEnd) <= std::distance(iterIni, iterEnd));
	// copy return data points
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
