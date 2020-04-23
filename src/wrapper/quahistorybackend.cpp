#include "quahistorybackend.h"

#include "quaserver_anex.h"

#ifdef UA_ENABLE_HISTORIZING

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

QUaHistoryDataPoint QUaHistoryBackend::dataValueToPoint(const UA_DataValue* value)
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

UA_DataValue QUaHistoryBackend::dataPointToValue(const QUaHistoryDataPoint* point)
{
	UA_DataValue retVal;
	// set values
	retVal.value = QUaTypesConverter::uaVariantFromQVariant(point->value);
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_DateTime, QDateTime>(point->timestamp, &retVal.serverTimestamp);
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_DateTime, QDateTime>(point->timestamp, &retVal.sourceTimestamp);
	retVal.status = point->status;
	retVal.serverPicoseconds = 0;
	retVal.sourcePicoseconds = 0;
	// let know it has htem
	retVal.hasValue = true;
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
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
			*nodeId,
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
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
			*nodeId,
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
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
			*nodeId,
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
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
		const UA_DateTime   timestamp,
		const MatchStrategy strategy) -> size_t
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QUaNodeId nodeIdQt = *nodeId;
		QDateTime time = timestamp == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&timestamp);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// check if exact time stamp exists in history database
		bool hasTimestamp = time.isValid() ? srv->m_historBackend.hasTimestamp(
			nodeIdQt,
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
			nodeIdQt,
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
		size_t           startIndex,
		size_t           endIndex) -> size_t
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QUaNodeId nodeIdQt  = *nodeId;
		QDateTime timeStart = startIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(startIndex);
		QDateTime timeEnd = endIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(endIndex);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		Q_ASSERT_X(
			timeStart.isValid() && srv->m_historBackend.hasTimestamp(nodeIdQt, timeStart, logOut),
			"QUaHistoryBackend::resultSize",
			"Error; startIndex not found"
		);
		Q_ASSERT_X(
			timeEnd.isValid() && srv->m_historBackend.hasTimestamp(nodeIdQt, timeEnd, logOut) ? true : endIndex == LLONG_MAX,
			"QUaHistoryBackend::resultSize",
			"Error; endIndex not found");
		// get number of data points in time range
		auto res = static_cast<size_t>(srv->m_historBackend.numDataPointsInRange(nodeIdQt, timeStart, timeEnd, logOut));
		QUaHistoryBackend::processServerLog(srv, logOut);
		return res;
	};
	// 8) It copies data values inside a certain range into a buffer.
	result.copyDataValues = [](
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
		size_t               startIndex,                // [IN] index of the first value in the range.
		size_t               endIndex,                  // [IN] index of the last value in the range.
		UA_Boolean           reverse,                   // [IN] if the values shall be copied in reverse order.
		size_t               valueSize,                 // [IN] maximal number of data values to copy.
		UA_NumericRange      range,                     // [IN] numeric range which shall be copied for every data value.
		UA_Boolean           releaseContinuationPoints, // [IN] if the continuation points shall be released. (not used in memory example?)
		const UA_ByteString* continuationPoint,         // [IN] point the client wants to release or start from.
		UA_ByteString* outContinuationPoint,      // [OUT] point which will be passed to the client.
		size_t* providedValues,            // [OUT] number of values that were copied.
		UA_DataValue* values) -> UA_StatusCode   // [OUT] values that have been copied from the database.
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		Q_UNUSED(releaseContinuationPoints); // not used?
		// convert inputs
		QUaNodeId nodeIdQt = *nodeId;
		QDateTime timeStart = startIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(startIndex);
		QDateTime timeEnd = endIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(endIndex);
		// get offset wrt to previous call
		QDateTime timeStartOffset = timeStart;
		if (continuationPoint->length > 0)
		{
			Q_ASSERT(continuationPoint->length == sizeof(size_t));
			if (continuationPoint->length != sizeof(size_t))
			{
				return UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
			}
			timeStartOffset = QDateTime::fromMSecsSinceEpoch(*((size_t*)(continuationPoint->data)));
			Q_ASSERT(timeStartOffset > timeStart);
		}
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// TODO : swap timestamps if reverse?
		if (reverse)
		{
			Q_ASSERT(timeEnd <= timeStartOffset);
			auto timeTmp = timeEnd;
			timeEnd = timeStartOffset;
			timeStartOffset = timeTmp;
		}
		else
		{
			Q_ASSERT(timeStartOffset <= timeEnd);
		}
		Q_ASSERT_X(
			timeStartOffset.isValid() && srv->m_historBackend.hasTimestamp(nodeIdQt, timeStartOffset, logOut),
			"QUaHistoryBackend::copyDataValues",
			"Error; startIndex not found"
		);
		Q_ASSERT_X(
			!timeEnd.isValid() || srv->m_historBackend.hasTimestamp(nodeIdQt, timeEnd, logOut),
			"QUaHistoryBackend::copyDataValues",
			"Error; invalid endIndex"
		);
		// read data, points must always come back in incresing timestamp order
		QVector<QUaHistoryDataPoint> points = srv->m_historBackend.readHistoryData(
			nodeIdQt,
			timeStartOffset,
			static_cast<quint64>(valueSize + 1), // NOTE : ask one more to get correct continuation point
			logOut
		);
		QUaHistoryBackend::processServerLog(srv, logOut);
		// unexpected size considered error
		if (valueSize + 1 != static_cast<size_t>(points.count()))
		{
			*providedValues = 0;
			return UA_STATUSCODE_BADUNEXPECTEDERROR;
		}
		// set provided values
		*providedValues = valueSize;
		// copy data
		auto iterIni = !reverse ? points.begin() : points.end() - 2/*1*/; // NOTE : -2 because of the continuation point thing
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
		// extra requested point is to get timestamp as next continuation point
		QDateTime timeStartOffsetNext = points.last().timestamp;
		if (timeStartOffsetNext.isValid())
		{
			Q_ASSERT(timeStartOffsetNext > timeStartOffset);
			outContinuationPoint->length = sizeof(size_t);
			size_t t = sizeof(size_t);
			outContinuationPoint->data = (UA_Byte*)UA_malloc(t);
			*((size_t*)(outContinuationPoint->data)) = timeStartOffsetNext.toMSecsSinceEpoch();
		}
		// success
		return UA_STATUSCODE_GOOD;
	};

	// Not Called from UaExpert : Returns the data value stored at a certain index in the database
	result.getDataValue = [](
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
		size_t           index) -> const UA_DataValue*
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QUaNodeId nodeIdQt = *nodeId;
		QDateTime time = index == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(index);
		Q_ASSERT(time.isValid());
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// read data, reusing existing API
		QVector<QUaHistoryDataPoint> points = srv->m_historBackend.readHistoryData(
			nodeIdQt,
			time,
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
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
		QUaNodeId nodeIdQt = *nodeId;
		auto    dataPoint = QUaHistoryBackend::dataValueToPoint(value);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// call internal backend method
		if (!srv->m_historBackend.writeHistoryData(
			nodeIdQt,
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
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
		const UA_DataValue* value) -> UA_StatusCode
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QUaNodeId nodeIdQt = *nodeId;
		auto    dataPoint = QUaHistoryBackend::dataValueToPoint(value);
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// call internal backend method
		if (!srv->m_historBackend.updateHistoryData(
			nodeIdQt,
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
	result.updateDataValue = updateHistoryData;
	result.replaceDataValue = updateHistoryData;

	// Not Called from UaExpert : update at given index, given index should exist, else use insert
	result.removeDataValue = [](
		UA_Server* server,
		void* hdbContext,
		const UA_NodeId* sessionId,
		void* sessionContext,
		const UA_NodeId* nodeId,
		UA_DateTime      startTimestamp,
		UA_DateTime      endTimestamp) -> UA_StatusCode
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		QUaNodeId nodeIdQt = *nodeId;
		QDateTime timeStart = startTimestamp == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&startTimestamp);
		QDateTime timeEnd = endTimestamp == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&endTimestamp);
		Q_ASSERT(timeStart.isValid());
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		Q_ASSERT_X(
			timeStart.isValid() && srv->m_historBackend.hasTimestamp(nodeIdQt, timeStart, logOut),
			"QUaHistoryBackend::removeDataValue",
			"Error; startIndex not found"
		);
		Q_ASSERT_X(
			!timeEnd.isValid() || srv->m_historBackend.hasTimestamp(nodeIdQt, timeEnd, logOut),
			"QUaHistoryBackend::removeDataValue",
			"Error; invalid endIndex"
		);
		// call internal backend method
		if (!srv->m_historBackend.removeHistoryData(
			nodeIdQt,
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
	m_writeHistoryData = nullptr;
	m_updateHistoryData = nullptr;
	m_removeHistoryData = nullptr;
	m_firstTimestamp = nullptr;
	m_lastTimestamp = nullptr;
	m_hasTimestamp = nullptr;
	m_findTimestamp = nullptr;
	m_numDataPointsInRange = nullptr;
	m_readHistoryData = nullptr;
}

bool QUaHistoryBackend::writeHistoryData(
	const QUaNodeId& nodeId,
	const QUaHistoryDataPoint& dataPoint,
	QQueue<QUaLog>& logOut)
{
	if (!m_writeHistoryData)
	{
		return false;
	}
	return m_writeHistoryData(nodeId, dataPoint, logOut);
}

bool QUaHistoryBackend::updateHistoryData(
	const QUaNodeId& nodeId,
	const QUaHistoryDataPoint& dataPoint,
	QQueue<QUaLog>& logOut)
{
	if (!m_updateHistoryData)
	{
		return false;
	}
	return m_updateHistoryData(nodeId, dataPoint, logOut);
}

bool QUaHistoryBackend::removeHistoryData(
	const QUaNodeId& nodeId,
	const QDateTime& timeStart,
	const QDateTime& timeEnd,
	QQueue<QUaLog>& logOut)
{
	if (!m_removeHistoryData)
	{
		return false;
	}
	return m_removeHistoryData(nodeId, timeStart, timeEnd, logOut);
}

QDateTime QUaHistoryBackend::firstTimestamp(
	const QUaNodeId& nodeId,
	QQueue<QUaLog>& logOut
) const
{
	if (!m_firstTimestamp)
	{
		return QDateTime();
	}
	return m_firstTimestamp(nodeId, logOut);
}

QDateTime QUaHistoryBackend::lastTimestamp(
	const QUaNodeId& nodeId,
	QQueue<QUaLog>& logOut
) const
{
	if (!m_lastTimestamp)
	{
		return QDateTime();
	}
	return m_lastTimestamp(nodeId, logOut);
}

bool QUaHistoryBackend::hasTimestamp(
	const QUaNodeId& nodeId,
	const QDateTime& timestamp,
	QQueue<QUaLog>& logOut
) const
{
	if (!m_hasTimestamp)
	{
		return false;
	}
	return m_hasTimestamp(nodeId, timestamp, logOut);
}

QDateTime QUaHistoryBackend::findTimestamp(
	const QUaNodeId& nodeId,
	const QDateTime& timestamp,
	const TimeMatch& match,
	QQueue<QUaLog>& logOut
) const
{
	if (!m_findTimestamp)
	{
		return QDateTime();
	}
	return m_findTimestamp(nodeId, timestamp, match, logOut);
}

quint64 QUaHistoryBackend::numDataPointsInRange(
	const QUaNodeId& nodeId,
	const QDateTime& timeStart,
	const QDateTime& timeEnd,
	QQueue<QUaLog>& logOut
) const
{
	if (!m_numDataPointsInRange)
	{
		return 0;
	}
	return m_numDataPointsInRange(nodeId, timeStart, timeEnd, logOut);
}

QVector<QUaHistoryDataPoint>
QUaHistoryBackend::readHistoryData(
	const QUaNodeId& nodeId,
	const QDateTime& timeStart,
	const quint64& numPointsToRead,
	QQueue<QUaLog>& logOut) const
{
	if (!m_readHistoryData)
	{
		return QVector<QUaHistoryDataPoint>();
	}
	return m_readHistoryData(
		nodeId,
		timeStart,
		numPointsToRead,
		logOut
	);
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

bool QUaHistoryBackend::writeHistoryEventsOfType(
	const QUaNodeId            &eventTypeNodeId,
	const QVector<QUaNodeId>   &emittersNodeIds,
	const QUaHistoryEventPoint &eventPoint,
	QQueue<QUaLog>             &logOut
)
{
	if (!m_writeHistoryEventsOfType)
	{
		return false;
	}
	return m_writeHistoryEventsOfType(
		eventTypeNodeId,
		emittersNodeIds,
		eventPoint,
		logOut
	);
}

QVector<QUaNodeId> QUaHistoryBackend::eventTypesOfEmitter(
	const QUaNodeId &emitterNodeId, 
	QQueue<QUaLog>  &logOut
)
{
	if (!m_eventTypesOfEmitter)
	{
		return QVector<QUaNodeId>();
	}
	return m_eventTypesOfEmitter(
		emitterNodeId,
		logOut
	);
}

QDateTime QUaHistoryBackend::findTimestampEventOfType(
	const QUaNodeId                    &emitterNodeId,
	const QUaNodeId                    &eventTypeNodeId,
	const QDateTime                    &timestamp,
	const QUaHistoryBackend::TimeMatch &match,
	QQueue<QUaLog>                     &logOut
)
{
	if (!m_findTimestampEventOfType)
	{
		return QDateTime();
	}
	return m_findTimestampEventOfType(
		emitterNodeId,
		eventTypeNodeId,
		timestamp,
		match,
		logOut
	);
}

quint64 QUaHistoryBackend::numEventsOfTypeInRange(
	const QUaNodeId &emitterNodeId,
	const QUaNodeId &eventTypeNodeId,
	const QDateTime &timeStart,
	const QDateTime &timeEnd,
	QQueue<QUaLog>  &logOut
)
{
	if (!m_numEventsOfTypeInRange)
	{
		return 0;
	}
	return m_numEventsOfTypeInRange(
		emitterNodeId,
		eventTypeNodeId,
		timeStart,
		timeEnd,
		logOut
	);
}

QVector<QUaHistoryEventPoint> QUaHistoryBackend::readHistoryEventsOfType(
	const QUaNodeId &emitterNodeId,
	const QUaNodeId &eventTypeNodeId,
	const QDateTime &timeStart,
	const quint64   &numPointsOffset,
	const quint64   &numPointsToRead,
	QQueue<QUaLog>  &logOut
)
{
	if (!m_readHistoryEventsOfType)
	{
		return QVector<QUaHistoryEventPoint>();
	}
	return m_readHistoryEventsOfType(
		emitterNodeId,
		eventTypeNodeId,
		timeStart,
		numPointsOffset,
		numPointsToRead,
		logOut
	);
}

bool QUaHistoryBackend::setEvent(
	QUaServer* server,
	const QUaNodeId& eventTypeNodeId,
	const QVector<QUaNodeId>& emittersNodeIds,
	const QUaHistoryEventPoint& eventPoint)
{
	QQueue<QUaLog> logOut;
	bool ok = server->m_historBackend.writeHistoryEventsOfType(
		eventTypeNodeId,
		emittersNodeIds,
		eventPoint,
		logOut
	);
	// TODO : process log
	return ok;
}

// based on readRaw_service_default
void QUaHistoryBackend::readEvent(
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
    UA_HistoryEvent* const* const historyData)
{
	Q_UNUSED(hdbContext);
	Q_UNUSED(sessionId);
	Q_UNUSED(sessionContext);
	Q_UNUSED(requestHeader);
	Q_UNUSED(timestampsToReturn);
	Q_UNUSED(releaseContinuationPoints);
	QQueue<QUaLog> logOut;
	auto srv = QUaServer::getServerNodeContext(server);
	// max number of events to read for each emitter
	// TODO : add a member to quaBaseEvent to explicit history enable
	// TODO : make maxInternal configurable, also rethink for data history as well
	// TODO : bug when no data requested time range, now it returns first point
	const quint64 maxInternal = 500;
	quint64 maxPerEmitter = (std::min)(
		maxInternal, 
		static_cast<quint64>(historyReadDetails->numValuesPerNode)
	);
	// get time range to read for each emitter
	auto startTimestamp = historyReadDetails->startTime;
	auto endTimestamp   = historyReadDetails->endTime;
	QDateTime timeStart = startTimestamp == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&startTimestamp);
	QDateTime timeEnd   = endTimestamp   == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&endTimestamp);
	// get qualnames for columns in advance
	size_t numCols = historyReadDetails->filter.selectClausesSize;
	QVector<QUaQualifiedName> colNames;
	colNames.resize(static_cast<int>(numCols));
	for (size_t col = 0; col < numCols; ++col)
	{
		auto sao = &historyReadDetails->filter.selectClauses[col];
		if (sao->browsePathSize == 0)
		{
			colNames[static_cast<int>(col)] = "EventNodeId";
			continue;
		}
		colNames[static_cast<int>(col)] = *sao->browsePath;
	}
	// loop all emitter nodes for which event history was requested
	for (size_t ithNode = 0; ithNode < nodesToReadSize; ++ithNode) 
	{
		// check if any events for given emitter
		QUaNodeId emitterNodeId = QUaNodeId(nodesToRead[ithNode].nodeId);
		// loop event types and populate
		QUaEventHistoryContinuationPoint queryData;
		// check if continuation point for given emitter is valid
		auto continuation = QUaEventHistoryQueryData::ContinuationFromUaByteString(
			nodesToRead[ithNode].continuationPoint
		);
		if (!continuation.isEmpty())
		{
			queryData = continuation;
		}
		else
		{
			// if no valid continuation point, then compute it
			QVector<QUaNodeId> evtTypeNodeIds = srv->m_historBackend.eventTypesOfEmitter(
				emitterNodeId,
				logOut
			);
			// TODO : process log
			for (auto evtTypeNodeId : evtTypeNodeIds)
			{
				QDateTime timeStartExisting = srv->m_historBackend.findTimestampEventOfType(
					emitterNodeId,
					evtTypeNodeId,
					timeStart,
					TimeMatch::ClosestFromAbove,
					logOut
				);
				// TODO : process log
				QDateTime timeEndExisting = srv->m_historBackend.findTimestampEventOfType(
					emitterNodeId,
					evtTypeNodeId,
					timeEnd,
					TimeMatch::ClosestFromBelow,
					logOut
				);
				// TODO : process log
				quint64 numEventsToRead = srv->m_historBackend.numEventsOfTypeInRange(
					emitterNodeId,
					evtTypeNodeId,
					timeStartExisting,
					timeEndExisting,
					logOut
				);
				// TODO : process log
				queryData[evtTypeNodeId] = {
					timeStartExisting,
					numEventsToRead
				};
			}
			auto test = QUaEventHistoryQueryData::ContinuationToByteArray(queryData);
			auto copy = QUaEventHistoryQueryData::ContinuationFromByteArray(test);
			Q_ASSERT(copy == queryData);
		}
		// calculate absolute total, total already read and total missing to read
		quint64 totalMissingToRead = 0;
		quint64 totalToReadInThisCall = 0;
		quint64 totalAlreadyReadInThisCall = 0;
		QList<QUaNodeId> evtTypeNodeIds = queryData.keys();
		for (auto& evtTypeNodeId : evtTypeNodeIds)
		{
			totalMissingToRead += queryData[evtTypeNodeId].m_numEventsToRead;
		}
		Q_ASSERT(totalMissingToRead >= 0);
		totalToReadInThisCall = (std::min)(maxPerEmitter, totalMissingToRead);
		// alloc output in qt format
		QVector<QUaHistoryEventPoint> allEvents;
		allEvents.resize(totalToReadInThisCall);
		for (auto &evtTypeNodeId : evtTypeNodeIds)
		{
			quint64 totalToReadForThisType =
				queryData[evtTypeNodeId].m_numEventsToRead;
			Q_ASSERT(totalToReadForThisType >= 0);
			if (totalToReadForThisType == 0)
			{
				continue;
			}
			// limit
			totalToReadForThisType = (std::min)(totalToReadForThisType, totalToReadInThisCall);
			Q_ASSERT(totalToReadForThisType > 0);
			// read output for current event type
			// TODO ; due to bug explained below, we might have to keep the m_timeStartExisting
			//        fixed and use an offset to keep track of already read values
			// https://www.sqlitetutorial.net/sqlite-window-functions/sqlite-row_number/
			// (Using SQLite ROW_NUMBER() for pagination)
			// https://www.sqlitetutorial.net/sqlite-limit/
			auto eventsOfType = srv->m_historBackend.readHistoryEventsOfType(
				emitterNodeId,
				evtTypeNodeId,
				queryData[evtTypeNodeId].m_timeStartExisting,
				queryData[evtTypeNodeId].m_numEventsAlreadyRead, // offset
				totalToReadForThisType,
				logOut
			);
			// TODO : process log
			Q_ASSERT_X(eventsOfType.size() == totalToReadForThisType, "TODO", "Create an error log");
			// update continuation
			queryData[evtTypeNodeId].m_numEventsToRead      -= totalToReadForThisType;
			queryData[evtTypeNodeId].m_numEventsAlreadyRead += totalToReadForThisType;
			if (queryData[evtTypeNodeId].m_numEventsToRead == 0)
			{
				queryData.remove(evtTypeNodeId);
			}
			Q_ASSERT(eventsOfType.size() == totalToReadForThisType);
			std::copy(eventsOfType.begin(), eventsOfType.end(), allEvents.begin() + totalAlreadyReadInThisCall);
			totalAlreadyReadInThisCall += totalToReadForThisType;
			Q_ASSERT(totalAlreadyReadInThisCall <= totalToReadInThisCall);
			if (totalAlreadyReadInThisCall == totalToReadInThisCall)
			{
				break;
			}
		}
		// update continuation
		response->results[ithNode].continuationPoint = queryData.isEmpty() ?
			UA_BYTESTRING_NULL :
			QUaEventHistoryQueryData::ContinuationToUaByteString(queryData);
		// alloc output all rows
		size_t numRows = allEvents.size();
		auto   iterRow = allEvents.begin();
		Q_ASSERT(numRows == totalToReadInThisCall);
		historyData[ithNode]->eventsSize = numRows;
		historyData[ithNode]->events = (UA_HistoryEventFieldList*)
			UA_Array_new(numRows, &UA_TYPES[UA_TYPES_HISTORYEVENTFIELDLIST]);
		// loop all rows
		for (size_t row = 0; row < numRows; row++)
		{
			// alloc output one row, all cols
			historyData[ithNode]->events[row].eventFieldsSize = numCols;
			historyData[ithNode]->events[row].eventFields = (UA_Variant*)
				UA_Array_new(numCols, &UA_TYPES[UA_TYPES_VARIANT]);
			for (size_t col = 0; col < numCols; ++col)
			{
				QUaQualifiedName& colName = colNames[static_cast<int>(col)];
				if (iterRow->fields.contains(colName))
				{
					historyData[ithNode]->events[row].eventFields[col] = 
						QUaTypesConverter::uaVariantFromQVariant(iterRow->fields.value(colName));
				}
				else
				{
					UA_Variant_init(&historyData[ithNode]->events[row].eventFields[col]);
				}
			}
			// inc row
			iterRow++;
		}
	} // end nodesToReadSize
	response->responseHeader.serviceResult = UA_STATUSCODE_GOOD;
}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // UA_ENABLE_HISTORIZING


