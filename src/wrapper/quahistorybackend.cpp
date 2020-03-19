#include "quahistorybackend.h"

#include <QUaServer>

UA_StatusCode UA_DataValue_backend_copyRange(
	const UA_DataValue &src, 
	UA_DataValue &dst, 
	const UA_NumericRange &range)
{
	memcpy(&dst, &src, sizeof(UA_DataValue));
	if (src.hasValue)
		return UA_Variant_copyRange(&src.value, &dst.value, range);
	return UA_STATUSCODE_BADDATAUNAVAILABLE;
}

UA_HistoryDataBackend QUaHistoryBackend::m_historUaBackend = QUaHistoryBackend::CreateUaBackend();

QUaHistoryBackend::DataPoint QUaHistoryBackend::dataValueToPoint(const UA_DataValue* value)
{
	// get or create timestamp for new point
	UA_DateTime timestamp = 0;
	if (value->hasSourceTimestamp)
	{
		timestamp = value->sourceTimestamp;
	}
	else if (value->hasServerTimestamp)
	{
		timestamp = value->serverTimestamp;
	}
	else
	{
		timestamp = UA_DateTime_now();
	}
	return {
			QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&timestamp),
			QUaTypesConverter::uaVariantToQVariant(value->value),
			value->status
	};
}

UA_DataValue QUaHistoryBackend::dataPointToValue(const DataPoint * point)
{
	UA_DataValue retVal;
	// set values
	retVal.value = QUaTypesConverter::uaVariantFromQVariant(point->value);
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_DateTime, QDateTime>(point->timestamp, &retVal.serverTimestamp);
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_DateTime, QDateTime>(point->timestamp, &retVal.sourceTimestamp);
	retVal.status = point->status;
	retVal.serverPicoseconds    = 0;
	retVal.hasSourcePicoseconds = 0;
	// let know it has htem
	retVal.hasValue  = true;
	retVal.hasStatus = true;
	retVal.hasServerTimestamp = true;
	retVal.hasSourceTimestamp = true;
	retVal.hasServerPicoseconds = false;
	retVal.hasSourcePicoseconds = false;
	return retVal;
}

void QUaHistoryBackend::processServerLog(
	QUaServer* server, 
	QQueue<QUaLog>& logOut
)
{
	while (logOut.count() > 0)
	{
		emit server->logMessage(logOut.dequeue());
	}
}

UA_HistoryDataBackend QUaHistoryBackend::CreateUaBackend()
{
	UA_HistoryDataBackend result;
	memset(&result, 0, sizeof(UA_HistoryDataBackend));
	// 0) This function sets a DataValue for a node in the historical data storage.
	result.serverSetHistoryData = [](
		UA_Server*          server,
		void*               hdbContext,
		const UA_NodeId*    sessionId,
		void*               sessionContext,
		const UA_NodeId*    nodeId,
		UA_Boolean          historizing,
		const UA_DataValue* value) -> UA_StatusCode
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionContext);
		// If sessionId is NULL, the historizing flag is invalid
		if (sessionId && !historizing)
		{
			return UA_STATUSCODE_BADNOTIMPLEMENTED;
		}
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// call internal backend method
		if (!srv->m_historBackend.writeHistoryData(
			QUaTypesConverter::nodeIdToQString(*nodeId),
			dataValueToPoint(value), 
			logOut
		))
		{
			QUaHistoryBackend::processServerLog(srv, logOut);
			return UA_STATUSCODE_BADUNEXPECTEDERROR;
		}
		QUaHistoryBackend::processServerLog(srv, logOut);
		return UA_STATUSCODE_GOOD;
	};
	// 1) This function returns UA_TRUE if the backend supports returning bounding values for a node. This function is mandatory.
	result.boundSupported = [](
		UA_Server*       server,
		void*            hdbContext,
		const UA_NodeId* sessionId,
		void*            sessionContext,
		const UA_NodeId* nodeId) -> UA_Boolean
	{
		Q_UNUSED(server);
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		Q_UNUSED(nodeId);
		// simplify API by returning always true
		return true;
	};
	// 2) This function returns UA_TRUE if the backend supports returning the requested timestamps for a node. This function is mandatory.
	result.timestampsToReturnSupported = [](
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
		const UA_TimestampsToReturn timestampsToReturn) -> UA_Boolean
	{
		Q_UNUSED(server);
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		Q_UNUSED(nodeId);
		Q_UNUSED(timestampsToReturn);
		// simplify API by supporting all
		return true;
	};
	// 3) It returns the index of the element after the last valid entry in the database for a node 
	//    (return index value considered to be invalid).
	result.getEnd = [](
		UA_Server*       server,
		void*            hdbContext,
		const UA_NodeId* sessionId,
		void*            sessionContext,
		const UA_NodeId* nodeId) -> size_t
	{
		Q_UNUSED(server);
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		Q_UNUSED(nodeId);
		// simplify API by returning always LLONG_MAX
		return LLONG_MAX;
	};
	// 4) It returns the index of the first element in the database for a node.
	result.firstIndex = [](
		UA_Server*       server,
		void*            hdbContext,
		const UA_NodeId* sessionId,
		void*            sessionContext,
		const UA_NodeId* nodeId) -> size_t
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// simplify API by considering that the timestamp is the index
		QDateTime time = srv->m_historBackend.firstTimestamp(
			QUaTypesConverter::nodeIdToQString(*nodeId), 
			logOut
		);
		QUaHistoryBackend::processServerLog(srv, logOut);
		// check
		if (!time.isValid())
		{
			return LLONG_MAX;
		}
		// return first available timestamp as index
		return static_cast<size_t>(time.toMSecsSinceEpoch());
	};
	// 5) It returns the index of the last element in the database for a node.
	result.lastIndex = [](
		UA_Server*       server,
		void*            hdbContext,
		const UA_NodeId* sessionId,
		void*            sessionContext,
		const UA_NodeId* nodeId) -> size_t
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// simplify API by considering that the timestamp is the index
		QDateTime time = srv->m_historBackend.lastTimestamp(
			QUaTypesConverter::nodeIdToQString(*nodeId), 
			logOut
		);
		QUaHistoryBackend::processServerLog(srv, logOut);
		// check
		if (!time.isValid())
		{
			return LLONG_MAX;
		}
		// return first available timestamp as index
		return static_cast<size_t>(time.toMSecsSinceEpoch());
	};
	// 6) It returns the index of a value in the database which matches certain criteria.
	result.getDateTimeMatch = [](UA_Server* server,
		void*               hdbContext,
		const UA_NodeId*    sessionId,
		void*               sessionContext,
		const UA_NodeId*    nodeId,
		const UA_DateTime   timestamp,
		const MatchStrategy strategy) -> size_t
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QString strNodeId = QUaTypesConverter::nodeIdToQString(*nodeId);
		QDateTime time = timestamp == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&timestamp);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// check if exact time stamp exists in history database
		bool hasTimestamp = time.isValid() ? srv->m_historBackend.hasTimestamp(
			strNodeId,
			time, 
			logOut
		) : false;
		// simplify API by simplifying the match
		if ((strategy == MATCH_EQUAL || strategy == MATCH_EQUAL_OR_AFTER || strategy == MATCH_EQUAL_OR_BEFORE)
			&& hasTimestamp)
		{
			QUaHistoryBackend::processServerLog(srv, logOut);
			return static_cast<size_t>(time.toMSecsSinceEpoch());
		}
		// get match type
		TimeMatch match;
		switch (strategy) {
		case MATCH_EQUAL_OR_AFTER:
		case MATCH_AFTER:
		{
			// contains(timestamp) is handled before, so closest from above
			match = TimeMatch::ClosestFromAbove;
		}
		break;
		case MATCH_EQUAL_OR_BEFORE:
		case MATCH_BEFORE:
		{
			// contains(timestamp) is handled before, so closest from below
			match = TimeMatch::ClosestFromBelow;
		}
		break;
		default:
			Q_ASSERT(false);
			break;
		}
		// find timestamp
		QDateTime outTime = srv->m_historBackend.findTimestamp(
			strNodeId,
			time,
			match, 
			logOut
		);
		QUaHistoryBackend::processServerLog(srv, logOut);
		// check
		if (!outTime.isValid())
		{
			return LLONG_MAX;
		}
		// return first available timestamp as index
		return static_cast<size_t>(outTime.toMSecsSinceEpoch());
	};
	// 7) It returns the number of elements between startIndex and endIndex including both.
	result.resultSize = [](
		UA_Server*       server,
		void*            hdbContext,
		const UA_NodeId* sessionId,
		void*            sessionContext,
		const UA_NodeId* nodeId,
		size_t           startIndex,
		size_t           endIndex) -> size_t
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QString   strNodeId = QUaTypesConverter::nodeIdToQString(*nodeId);
		QDateTime timeStart = startIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(startIndex);
		QDateTime timeEnd   = endIndex   == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(endIndex);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		Q_ASSERT_X(
			timeStart.isValid() && srv->m_historBackend.hasTimestamp(strNodeId, timeStart, logOut),
			"QUaHistoryBackend::resultSize",
			"Error; startIndex not found"
		);
		Q_ASSERT_X(
			timeEnd.isValid() && srv->m_historBackend.hasTimestamp(strNodeId, timeEnd, logOut) ? true : endIndex == LLONG_MAX,
			"QUaHistoryBackend::resultSize",
			"Error; endIndex not found");
		// get number of data points in time range
		auto res =  static_cast<size_t>(srv->m_historBackend.numDataPointsInRange(strNodeId, timeStart, timeEnd, logOut));
		QUaHistoryBackend::processServerLog(srv, logOut);
		return res;
	};
	// 8) It copies data values inside a certain range into a buffer.
	result.copyDataValues = [](
		UA_Server*           server,
		void*                hdbContext,
		const UA_NodeId*     sessionId,
		void*                sessionContext,
		const UA_NodeId*     nodeId,
		size_t               startIndex,                // [IN] index of the first value in the range.
		size_t               endIndex,                  // [IN] index of the last value in the range.
		UA_Boolean           reverse,                   // [IN] if the values shall be copied in reverse order.
		size_t               valueSize,                 // [IN] maximal number of data values to copy.
		UA_NumericRange      range,                     // [IN] numeric range which shall be copied for every data value.
		UA_Boolean           releaseContinuationPoints, // [IN] if the continuation points shall be released. (not used in memory example?)
		const UA_ByteString* continuationPoint,         // [IN] point the client wants to release or start from.
		UA_ByteString*       outContinuationPoint,      // [OUT] point which will be passed to the client.
		size_t*              providedValues,            // [OUT] number of values that were copied.
		UA_DataValue*        values) -> UA_StatusCode   // [OUT] values that have been copied from the database.
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		Q_UNUSED(releaseContinuationPoints); // not used?
		// get offset wrt to previous call
		size_t offset = 0;
		if (continuationPoint->length > 0)
		{
			Q_ASSERT(continuationPoint->length == sizeof(size_t));
			if (continuationPoint->length != sizeof(size_t))
			{
				return UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
			}
			offset = *((size_t*)(continuationPoint->data));
		}
		QString   strNodeId = QUaTypesConverter::nodeIdToQString(*nodeId);
		QDateTime timeStart = startIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(startIndex);
		QDateTime timeEnd   = endIndex   == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(endIndex);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		Q_ASSERT_X(
			timeStart.isValid() && srv->m_historBackend.hasTimestamp(strNodeId, timeStart, logOut),
			"QUaHistoryBackend::copyDataValues",
			"Error; startIndex not found"
		);
		Q_ASSERT_X(
			!timeEnd.isValid() || srv->m_historBackend.hasTimestamp(strNodeId, timeEnd, logOut),
			"QUaHistoryBackend::copyDataValues",
			"Error; invalid endIndex"
		);
		// TODO : swap timestamps if reverse?
		if (reverse)
		{
			Q_ASSERT(timeEnd <= timeStart);
			auto timeTmp = timeEnd;
			timeEnd   = timeStart;
			timeStart = timeTmp;
		}
		else
		{
			Q_ASSERT(timeStart <= timeEnd);
		}
		// read data
		QVector<DataPoint> points = srv->m_historBackend.readHistoryData(
			strNodeId,
			timeStart,
			timeEnd,
			static_cast<quint64>(offset),
			static_cast<quint64>(valueSize), 
			logOut
		);
		QUaHistoryBackend::processServerLog(srv, logOut);
		// unexpected size considered error
		if (valueSize != static_cast<size_t>(points.count()))
		{
			*providedValues = 0;
			return UA_STATUSCODE_BADUNEXPECTEDERROR;
		}
		// set provided values
		*providedValues = valueSize;
		// copy data
		auto iterIni = !reverse ? points.begin() : points.end() - 1;
		std::generate(values, values + valueSize,
			[&iterIni, &range, &reverse]() {
				UA_DataValue retVal;
				if (range.dimensionsSize > 0)
				{
					UA_DataValue_backend_copyRange(QUaHistoryBackend::dataPointToValue(iterIni), retVal, range);
				}
				else
				{
					retVal = QUaHistoryBackend::dataPointToValue(iterIni);
				}
				(!reverse) ? iterIni++ : iterIni--;
				return retVal;
			});
		// calculate next offset
		outContinuationPoint->length = sizeof(size_t);
		size_t t = sizeof(size_t);
		outContinuationPoint->data = (UA_Byte*)UA_malloc(t);
		*((size_t*)(outContinuationPoint->data)) = offset + valueSize;
		// success
		return UA_STATUSCODE_GOOD;
	};

	// Not Called from UaExpert : Returns the data value stored at a certain index in the database
	result.getDataValue = [](
		UA_Server*       server,
		void*            hdbContext,
		const UA_NodeId* sessionId,
		void*            sessionContext,
		const UA_NodeId* nodeId,
		size_t           index) -> const UA_DataValue*
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QString   strNodeId = QUaTypesConverter::nodeIdToQString(*nodeId);
		QDateTime time = index == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(index);
		Q_ASSERT(time.isValid());
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// read data, reusing existing API
		QVector<DataPoint> points = srv->m_historBackend.readHistoryData(
			strNodeId,
			time,
			time,
			0 /* 0 offset */,
			1 /* read just 1 value */, 
			logOut
		);
		QUaHistoryBackend::processServerLog(srv, logOut);
		// unexpected size considered error
		if (static_cast<size_t>(points.count()) != 1)
		{
			return nullptr;
		}
		// TODO : find better way to store the instance of the returned address
		static auto retVal = QUaHistoryBackend::dataPointToValue(points.begin());
		return &retVal;
	};

	// Not Called from UaExpert : insert at given index, given index should not yet exist, else use update
	result.insertDataValue = [](
		UA_Server*          server,
		void*               hdbContext,
		const UA_NodeId*    sessionId,
		void*               sessionContext,
		const UA_NodeId*    nodeId,
		const UA_DataValue* value) -> UA_StatusCode
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		// get or create timestamp for point
		if (!value->hasSourceTimestamp && !value->hasServerTimestamp)
		{
			return UA_STATUSCODE_BADINVALIDTIMESTAMP;
		}
		QString strNodeId = QUaTypesConverter::nodeIdToQString(*nodeId);
		auto    dataPoint = QUaHistoryBackend::dataValueToPoint(value);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// call internal backend method
		if (!srv->m_historBackend.writeHistoryData(
			QUaTypesConverter::nodeIdToQString(*nodeId),
			dataValueToPoint(value), 
			logOut
		))
		{
			QUaHistoryBackend::processServerLog(srv, logOut);
			return UA_STATUSCODE_BADUNEXPECTEDERROR;
		}
		QUaHistoryBackend::processServerLog(srv, logOut);
		return UA_STATUSCODE_GOOD;
	};

	// Not Called from UaExpert : update at given index, given index should exist, else use insert
	auto updateHistoryData = [](
		UA_Server*          server,
		void*               hdbContext,
		const UA_NodeId*    sessionId,
		void*               sessionContext,
		const UA_NodeId*    nodeId,
		const UA_DataValue* value) -> UA_StatusCode
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QString strNodeId = QUaTypesConverter::nodeIdToQString(*nodeId);
		auto    dataPoint = QUaHistoryBackend::dataValueToPoint(value);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// call internal backend method
		if (!srv->m_historBackend.updateHistoryData(
			QUaTypesConverter::nodeIdToQString(*nodeId),
			dataValueToPoint(value), 
			logOut
		))
		{
			QUaHistoryBackend::processServerLog(srv, logOut);
			return UA_STATUSCODE_BADUNEXPECTEDERROR;
		}
		QUaHistoryBackend::processServerLog(srv, logOut);
		return UA_STATUSCODE_GOOD;
	};
	result.updateDataValue  = updateHistoryData;
	result.replaceDataValue = updateHistoryData;

	// Not Called from UaExpert : update at given index, given index should exist, else use insert
	result.removeDataValue = [](
		UA_Server*       server,
		void*            hdbContext,
		const UA_NodeId* sessionId,
		void*            sessionContext,
		const UA_NodeId* nodeId,
		UA_DateTime      startTimestamp,
		UA_DateTime      endTimestamp) -> UA_StatusCode
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QString   strNodeId = QUaTypesConverter::nodeIdToQString(*nodeId);
		QDateTime timeStart = startTimestamp == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&startTimestamp);
		QDateTime timeEnd   = endTimestamp   == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&endTimestamp);
		Q_ASSERT(timeStart.isValid());
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		Q_ASSERT_X(
			timeStart.isValid() && srv->m_historBackend.hasTimestamp(strNodeId, timeStart, logOut),
			"QUaHistoryBackend::removeDataValue",
			"Error; startIndex not found"
		);
		Q_ASSERT_X(
			!timeEnd.isValid() || srv->m_historBackend.hasTimestamp(strNodeId, timeEnd, logOut),
			"QUaHistoryBackend::removeDataValue",
			"Error; invalid endIndex"
		);
		// call internal backend method
		if (!srv->m_historBackend.removeHistoryData(
			strNodeId,
			timeStart,
			timeEnd, 
			logOut
		))
		{
			QUaHistoryBackend::processServerLog(srv, logOut);
			return UA_STATUSCODE_BADUNEXPECTEDERROR;
		}
		QUaHistoryBackend::processServerLog(srv, logOut);
		return UA_STATUSCODE_GOOD;
	};

	// TODO : Not applicable?
	result.deleteMembers = nullptr;
	// This function is the high level interface for the ReadRaw operation. Set it to NULL if you use the low level API for your plugin.
	result.getHistoryData = nullptr;
	// Not used here
	result.context = nullptr;
	//
	return result;
}

QUaHistoryBackend::QUaHistoryBackend()
{
	m_writeHistoryData     = nullptr;
	m_updateHistoryData    = nullptr;
	m_removeHistoryData    = nullptr;
	m_firstTimestamp       = nullptr;
	m_lastTimestamp        = nullptr;
	m_hasTimestamp         = nullptr;
	m_findTimestamp        = nullptr;
	m_numDataPointsInRange = nullptr;
	m_readHistoryData      = nullptr;
}

bool QUaHistoryBackend::writeHistoryData(
	const QString   &strNodeId, 
	const DataPoint &dataPoint,
	QQueue<QUaLog>  &logOut)
{
	if (!m_writeHistoryData)
	{
		return false;
	}
	return m_writeHistoryData(strNodeId, dataPoint, logOut);
}

bool QUaHistoryBackend::updateHistoryData(
	const QString   &strNodeId, 
	const DataPoint &dataPoint,
	QQueue<QUaLog>  &logOut)
{
	if (!m_updateHistoryData)
	{
		return false;
	}
	return m_updateHistoryData(strNodeId, dataPoint, logOut);
}

bool QUaHistoryBackend::removeHistoryData(
	const QString   &strNodeId,
	const QDateTime &timeStart, 
	const QDateTime &timeEnd,
	QQueue<QUaLog>  &logOut)
{
	if (!m_removeHistoryData)
	{
		return false;
	}
	return m_removeHistoryData(strNodeId, timeStart, timeEnd, logOut);
}

QDateTime QUaHistoryBackend::firstTimestamp(
		const QString  &strNodeId,
		QQueue<QUaLog> &logOut
	) const
{
	if (!m_firstTimestamp)
	{
		return QDateTime();
	}
	return m_firstTimestamp(strNodeId, logOut);
}

QDateTime QUaHistoryBackend::lastTimestamp(
		const QString  &strNodeId,
		QQueue<QUaLog> &logOut
	) const
{
	if (!m_lastTimestamp)
	{
		return QDateTime();
	}
	return m_lastTimestamp(strNodeId, logOut);
}

bool QUaHistoryBackend::hasTimestamp(
		const QString   &strNodeId, 
		const QDateTime &timestamp,
		QQueue<QUaLog>  &logOut
	) const
{
	if (!m_hasTimestamp)
	{
		return false;
	}
	return m_hasTimestamp(strNodeId, timestamp, logOut);
}

QDateTime QUaHistoryBackend::findTimestamp(
		const QString   &strNodeId, 
		const QDateTime &timestamp, 
		const TimeMatch &match,
		QQueue<QUaLog>  &logOut
	) const
{
	if (!m_findTimestamp)
	{
		return QDateTime();
	}
	return m_findTimestamp(strNodeId, timestamp, match, logOut);
}

quint64 QUaHistoryBackend::numDataPointsInRange(
		const QString   &strNodeId, 
		const QDateTime &timeStart, 
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
	) const
{
	if (!m_numDataPointsInRange)
	{
		return 0;
	}
	return m_numDataPointsInRange(strNodeId, timeStart, timeEnd, logOut);
}

QVector<QUaHistoryBackend::DataPoint> 
QUaHistoryBackend::readHistoryData(
	const QString   &strNodeId, 
	const QDateTime &timeStart, 
	const QDateTime &timeEnd, 
	const quint64   &numPointsAlreadyRead, 
	const quint64   &numPointsToRead,
	QQueue<QUaLog>  &logOut) const
{
	if (!m_readHistoryData)
	{
		return QVector<QUaHistoryBackend::DataPoint>();
	}
	return m_readHistoryData(
		strNodeId,
		timeStart,
		timeEnd,
		numPointsAlreadyRead,
		numPointsToRead,
		logOut
	);
}


