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

QMetaType::Type QUaHistoryBackend::QVariantToQtType(const QVariant& value)
{
	return static_cast<QMetaType::Type>(
        value.type() < static_cast<QVariant::Type>(QMetaType::User) ?
		value.type() :
        static_cast<QVariant::Type>(value.userType())
	);
}

void QUaHistoryBackend::fixOutputVariantType(
	QVariant& value, 
	const QMetaType::Type& metaType)
{
	if (metaType == QMetaType::UnknownType || !value.isValid() || value.isNull())
	{
		return;
	}
	auto oldType = QUaHistoryBackend::QVariantToQtType(value);
	// if same, nothing to do
	if (oldType == metaType)
	{
		return;
	}
	// special cases
	if (metaType == QMetaType::QDateTime && value.canConvert(QMetaType::ULongLong))
	{
		qulonglong iTime = value.toULongLong();
		value = QDateTime::fromMSecsSinceEpoch(iTime, Qt::UTC);  // NOTE : expensive if spec not defined
		return;
	}
	if (metaType == QMetaType_StatusCode && value.canConvert(QMetaType::UInt))
	{
		uint iStatusCode = value.toUInt();
		value = QVariant::fromValue(static_cast<QUaStatusCode>(iStatusCode));
		return;
	}
	// generic case
	if (!value.canConvert(metaType))
	{
		//qWarning() << "[OLD TYPE]" << QMetaType::typeName(oldType);  
		//qWarning() << "[NEW TYPE]" << QMetaType::typeName(metaType); 
		//Q_ASSERT_X(false, "QUaHistoryBackend::fixOutputVariantType", "Cannot convert between types.");
		return;
	}
	// NOTE : expensive to convert QString to QUaNodeId
	value.convert(metaType); 
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
			// NOTE : inverted because we want points inside range, not outside
			// contains(timestamp) is handled before, so closest
			match = TimeMatch::ClosestFromBelow;
		}
		break;
		case MATCH_EQUAL_OR_BEFORE:
		case MATCH_BEFORE:
		{
			// NOTE : inverted because we want points inside range, not outside
			// contains(timestamp) is handled before, so closest
			match = TimeMatch::ClosestFromAbove;
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
		QDateTime timeStart = startIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(startIndex, Qt::UTC);
		QDateTime timeEnd = endIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(endIndex, Qt::UTC);
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
		UA_ByteString* outContinuationPoint,   // [OUT] point which will be passed to the client.
		size_t* providedValues,                // [OUT] number of values that were copied.
		UA_DataValue* values) -> UA_StatusCode // [OUT] values that have been copied from the database.
	{
		Q_UNUSED(hdbContext);
		Q_UNUSED(sessionId);
		Q_UNUSED(sessionContext);
		Q_UNUSED(releaseContinuationPoints); // not used?
		// convert inputs
		QUaNodeId nodeIdQt = *nodeId;
		QDateTime timeStart = startIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(startIndex, Qt::UTC);
		QDateTime timeEnd = endIndex == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(endIndex, Qt::UTC);
		// get offset wrt to previous call
		quint64 offset = 0;
		if (continuationPoint->length > 0)
		{
			Q_ASSERT(continuationPoint->length == sizeof(size_t));
			if (continuationPoint->length != sizeof(size_t))
			{
				return UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
			}
			offset = static_cast<quint64>(*((size_t*)(continuationPoint->data)));
		}
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// TODO : swap timestamps if reverse?
		if (reverse)
		{
			Q_ASSERT(timeEnd <= timeStart);
			auto timeTmp = timeEnd;
			timeEnd = timeStart;
			timeStart = timeTmp;
		}
		else
		{
			Q_ASSERT(timeStart <= timeEnd);
		}
		Q_ASSERT_X(
			timeStart.isValid() && srv->m_historBackend.hasTimestamp(nodeIdQt, timeStart, logOut),
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
			timeStart,
			offset,
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
		// calculate next continuation point
		offset += static_cast<quint64>(valueSize);

		outContinuationPoint->length = sizeof(size_t);
		size_t t = sizeof(size_t);
		outContinuationPoint->data = (UA_Byte*)UA_malloc(t);
		*((size_t*)(outContinuationPoint->data)) = static_cast<size_t>(offset);
		
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
		QDateTime time = index == LLONG_MAX ? QDateTime() : QDateTime::fromMSecsSinceEpoch(index, Qt::UTC);
		Q_ASSERT(time.isValid());
		// get server
		QQueue<QUaLog> logOut;
		QUaServer* srv = QUaServer::getServerNodeContext(server);
		// read data, reusing existing API
		QVector<QUaHistoryDataPoint> points = srv->m_historBackend.readHistoryData(
			nodeIdQt,
			time,
			0, /* no offset*/
			1 /* read just 1 value */,
			logOut
		);
		// unexpected size considered error
		if (static_cast<size_t>(points.count()) != 1)
		{
			QUaHistoryBackend::processServerLog(srv, logOut);
			return nullptr;
		}
		QUaHistoryBackend::processServerLog(srv, logOut);
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
	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	m_writeHistoryEventsOfType = nullptr;
	m_eventTypesOfEmitter = nullptr;
	m_findTimestampEventOfType = nullptr;
	m_numEventsOfTypeInRange = nullptr;
	m_readHistoryEventsOfType = nullptr;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
}

bool QUaHistoryBackend::writeHistoryData(
	const QUaNodeId& nodeId,
	const QUaHistoryDataPoint& dataPoint,
	QQueue<QUaLog>& logOut)
{
	if (!m_writeHistoryData)
	{
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
		return 0;
	}
	return m_numDataPointsInRange(nodeId, timeStart, timeEnd, logOut);
}

QVector<QUaHistoryDataPoint>
QUaHistoryBackend::readHistoryData(
	const QUaNodeId& nodeId,
	const QDateTime& timeStart,
	const quint64& numPointsOffset,
	const quint64& numPointsToRead,
	QQueue<QUaLog>& logOut) const
{
	if (!m_readHistoryData)
	{
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
		return QVector<QUaHistoryDataPoint>();
	}
	return m_readHistoryData(
		nodeId,
		timeStart,
		numPointsOffset,
		numPointsToRead,
		logOut
	);
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

bool QUaHistoryBackend::writeHistoryEventsOfType(
	const QUaNodeId            &eventTypeNodeId,
	const QList<QUaNodeId>   &emittersNodeIds,
	const QUaHistoryEventPoint &eventPoint,
	QQueue<QUaLog>             &logOut
)
{
	if (!m_writeHistoryEventsOfType)
	{
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
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
	const QList<QUaBrowsePath> &columnsToRead,
	QQueue<QUaLog>  &logOut
)
{
	if (!m_readHistoryEventsOfType)
	{
		logOut << QUaLog({
			QObject::tr("Historizing enabled, but historized not set."),
			QUaLogLevel::Warning,
			QUaLogCategory::History
		});
		return QVector<QUaHistoryEventPoint>();
	}
	return m_readHistoryEventsOfType(
		emitterNodeId,
		eventTypeNodeId,
		timeStart,
		numPointsOffset,
		numPointsToRead,
		columnsToRead,
		logOut
	);
}

bool QUaHistoryBackend::setEvent(
	QUaServer* server,
	const QUaNodeId& eventTypeNodeId,
	const QList<QUaNodeId>& emittersNodeIds,
	const QUaHistoryEventPoint& eventPoint)
{
	QQueue<QUaLog> logOut;
	bool ok = server->m_historBackend.writeHistoryEventsOfType(
		eventTypeNodeId,
		emittersNodeIds,
		eventPoint,
		logOut
	);
	QUaHistoryBackend::processServerLog(server, logOut);
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
	// merge request per node limit with internal server limit
	quint64 maxPerEmitterRequest = static_cast<quint64>(historyReadDetails->numValuesPerNode);
	quint64 maxPerEmitterServer  = srv->maxHistoryEventResponseSize();
	quint64 maxPerEmitter = maxPerEmitterRequest > 0 && maxPerEmitterServer > 0 ?
		(std::min)(maxPerEmitterServer, maxPerEmitterRequest) : 
		maxPerEmitterRequest > 0 ?
		maxPerEmitterServer :
		maxPerEmitterServer > 0 ?
		maxPerEmitterRequest :
		static_cast<quint64>((std::numeric_limits<quint32>::max)());
	// get time range to read for each emitter
	auto startTimestamp = historyReadDetails->startTime;
	auto endTimestamp   = historyReadDetails->endTime;
	QDateTime timeStart = startTimestamp == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&startTimestamp);
	QDateTime timeEnd   = endTimestamp   == LLONG_MAX ? QDateTime() : QUaTypesConverter::uaVariantToQVariantScalar<QDateTime, UA_DateTime>(&endTimestamp);
	// get qualnames for columns in advance
	size_t numCols = historyReadDetails->filter.selectClausesSize;
	QList<QUaBrowsePath> colBrowsePaths;
	for (size_t col = 0; col < numCols; ++col)
	{
		auto sao = &historyReadDetails->filter.selectClauses[col];
		if (sao->browsePathSize == 0)
		{
			const static auto eventNodeIdPath = QUaBrowsePath() << QUaQualifiedName(0, "EventNodeId");
			colBrowsePaths << eventNodeIdPath;
			continue;
		}
		colBrowsePaths << QUaQualifiedName::saoToBrowsePath(sao);
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
			QVector<QUaNodeId> eventTypeNodeIds = srv->m_historBackend.eventTypesOfEmitter(
				emitterNodeId,
				logOut
			);
			for (const auto & eventTypeNodeId : eventTypeNodeIds)
			{
				QDateTime timeStartExisting = srv->m_historBackend.findTimestampEventOfType(
					emitterNodeId,
					eventTypeNodeId,
					timeStart,
					TimeMatch::ClosestFromAbove,
					logOut
				);
				if (!timeStartExisting.isValid())
				{
					logOut << QUaLog({
						QObject::tr("Invalid start timestamp returned for events of type %1 for emitter %2.")
							.arg(eventTypeNodeId)
							.arg(emitterNodeId),
						QUaLogLevel::Warning,
						QUaLogCategory::History
					});
					continue;
				}
				if (timeStartExisting < timeStart)
				{
					// out of range
					continue;
				}
				QDateTime timeEndExisting = srv->m_historBackend.findTimestampEventOfType(
					emitterNodeId,
					eventTypeNodeId,
					timeEnd,
					TimeMatch::ClosestFromBelow,
					logOut
				);
				if (!timeEndExisting.isValid())
				{
					logOut << QUaLog({
						QObject::tr("Invalid end timestamp returned for events of type %1 for emitter %2.")
							.arg(eventTypeNodeId)
							.arg(emitterNodeId),
						QUaLogLevel::Warning,
						QUaLogCategory::History
					});
					continue;
				}
				if (timeEndExisting > timeEnd)
				{
					// out of range
					continue;
				}
				quint64 numEventsToRead = srv->m_historBackend.numEventsOfTypeInRange(
					emitterNodeId,
					eventTypeNodeId,
					timeStartExisting,
					timeEndExisting,
					logOut
				);
				if (numEventsToRead == 0)
				{
					continue;
				}
				queryData[eventTypeNodeId] = {
					timeStartExisting,
					numEventsToRead
				};
			}
		}
		// calculate absolute total, total already read and total missing to read
		quint64 totalMissingToRead = 0;
		quint64 totalToReadInThisCall = 0;
		quint64 totalAlreadyReadInThisCall = 0;
		QList<QUaNodeId> eventTypeNodeIds = queryData.keys();
		// early exit
		if (eventTypeNodeIds.isEmpty())
		{
			// next node to read
			continue;
		}
		for (auto& eventTypeNodeId : eventTypeNodeIds)
		{
			totalMissingToRead += queryData[eventTypeNodeId].m_numEventsToRead;
		}
		totalToReadInThisCall = (std::min)(maxPerEmitter, totalMissingToRead);
		// alloc output in qt format
		QVector<QUaHistoryEventPoint> allEvents;
		allEvents.resize(totalToReadInThisCall);
		for (auto &eventTypeNodeId : eventTypeNodeIds)
		{
			quint64 totalToReadForThisType =
				queryData[eventTypeNodeId].m_numEventsToRead;
			if (totalToReadForThisType == 0)
			{
				continue;
			}
			// limit
			Q_ASSERT(totalToReadInThisCall > totalAlreadyReadInThisCall);
			totalToReadForThisType = (std::min)(totalToReadForThisType, totalToReadInThisCall - totalAlreadyReadInThisCall);
			Q_ASSERT(totalToReadForThisType > 0);
			// read output for current event type
			auto eventsOfType = srv->m_historBackend.readHistoryEventsOfType(
				emitterNodeId,
				eventTypeNodeId,
				queryData[eventTypeNodeId].m_timeStartExisting,
				queryData[eventTypeNodeId].m_numEventsAlreadyRead, // offset
				totalToReadForThisType,
				colBrowsePaths,
				logOut
			);
			Q_ASSERT_X(
				eventsOfType.size() == totalToReadForThisType, 
				"readHistoryEventsOfType", 
				"readHistoryEventsOfType returned less values than requested"
			);
            if (static_cast<quint64>(eventsOfType.size()) != totalToReadForThisType)
			{
				logOut << QUaLog({
					QObject::tr("Reading historic events of type %1 for emitter %2 "
					"returned less values than requested. "
					"Returned (%3) != Requested (%4).")
						.arg(eventTypeNodeId)
						.arg(emitterNodeId)
						.arg(eventsOfType.size())
						.arg(totalToReadForThisType),
					QUaLogLevel::Warning,
					QUaLogCategory::History
				});
			}
			totalToReadForThisType = (std::min)(totalToReadForThisType, static_cast<quint64>(eventsOfType.size()));
			Q_ASSERT(totalToReadForThisType > 0);
			// update continuation
			queryData[eventTypeNodeId].m_numEventsToRead      -= totalToReadForThisType;
			queryData[eventTypeNodeId].m_numEventsAlreadyRead += totalToReadForThisType;
			if (queryData[eventTypeNodeId].m_numEventsToRead == 0)
			{
				queryData.remove(eventTypeNodeId);
			}
			// if the user returned non-matching qvariant types, they need fixing
			Q_ASSERT(srv->m_hashTypeVars.contains(eventTypeNodeId));
			auto& fieldInfo = srv->m_hashTypeVars[eventTypeNodeId];
			std::for_each(eventsOfType.begin(), eventsOfType.end(), [&fieldInfo](QUaHistoryEventPoint &point) {
				auto i = point.fields.begin();
				while (i != point.fields.end())
				{
					auto& name = i.key();
					QVariant& value = i.value();
					// NOTE : use ::value to avoid creating an unwanted entry into m_hashTypeVars
					auto &type = fieldInfo.value(name, QMetaType::UnknownType); 
					// NOTE : expensive, e.g. QString to QUaNodeId
					QUaHistoryBackend::fixOutputVariantType(value, type); 
					++i;
				}
			});
			// copy sub-vector to output vector
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
				auto& colPath = colBrowsePaths[static_cast<int>(col)];
				if (iterRow->fields.contains(colPath))
				{
					auto& value = iterRow->fields[colPath];
					if (!value.isValid())
					{
						continue;
					}
					historyData[ithNode]->events[row].eventFields[col] = 
						QUaTypesConverter::uaVariantFromQVariant(value);
				}
			}
			// inc row
			iterRow++;
		}
	} // end nodesToReadSize
	QUaHistoryBackend::processServerLog(srv, logOut);
	response->responseHeader.serviceResult = UA_STATUSCODE_GOOD;
}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // UA_ENABLE_HISTORIZING


