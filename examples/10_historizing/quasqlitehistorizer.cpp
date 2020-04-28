#include "quasqlitehistorizer.h"

#include <QUaServer>

#ifdef UA_ENABLE_HISTORIZING

#include <QSqlError>
#include <QSqlRecord>

// map supported types
QHash<int, QString> QUaSqliteHistorizer::m_hashTypes = {
	{QMetaType::Bool           , "INTEGER"},
	{QMetaType::Char           , "INTEGER"},
	{QMetaType::SChar          , "INTEGER"},
	{QMetaType::UChar          , "INTEGER"},
	{QMetaType::Short          , "INTEGER"},
	{QMetaType::UShort         , "INTEGER"},
	{QMetaType::Int            , "INTEGER"},
	{QMetaType::UInt           , "INTEGER"},
	{QMetaType::Long           , "INTEGER"},
	{QMetaType::LongLong       , "INTEGER"},
	{QMetaType::ULong          , "INTEGER"},
	{QMetaType::ULongLong      , "INTEGER"},
	{QMetaType::Float          , "REAL"   },
	{QMetaType::Double         , "REAL"   },
	{QMetaType::QString        , "TEXT"   },
	{QMetaType::QDateTime      , "INTEGER"},
	{QMetaType::QUuid          , "INTEGER"},
	{QMetaType::QByteArray     , "BLOB"   },
	{QMetaType::UnknownType    , "TEXT"   },
	{QMetaType_DataType        , "INTEGER"},
	{QMetaType_NodeId          , "TEXT"   },
	{QMetaType_StatusCode      , "TEXT"   },
	{QMetaType_QualifiedName   , "TEXT"   },
	{QMetaType_LocalizedText   , "TEXT"   },
	{QMetaType_List_NodeId       , "TEXT"   },
	{QMetaType_List_StatusCode   , "TEXT"   },
	{QMetaType_List_QualifiedName, "TEXT"   },
	{QMetaType_List_LocalizedText, "TEXT"   },
	{QMetaType_List_DataType     , "TEXT"   }
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	,
	{QMetaType_TimeZone                    , "TEXT"},
	{QMetaType_ChangeStructureDataType     , "TEXT"},
	{QMetaType_List_ChangeStructureDataType, "TEXT"}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

};

QUaSqliteHistorizer::QUaSqliteHistorizer()
{
	m_timeoutTransaction = 1000;
	QObject::connect(&m_timerTransaction, &QTimer::timeout, &m_timerTransaction,
	[this]() {
		// stop timer until next write request
		m_timerTransaction.stop();
		// commit transaction
		QSqlDatabase db;
		if (!this->getOpenedDatabase(db, m_deferedLogOut))
		{
			return;
		}
		if (!db.commit())
		{
			m_deferedLogOut << QUaLog({
				QObject::tr("Failed to commit transaction in %1 database. Sql : %2.")
					.arg(m_strSqliteDbName)
					.arg(db.lastError().text()),
				QUaLogLevel::Error,
				QUaLogCategory::History
			});
		}
	}, Qt::QueuedConnection);
}

QUaSqliteHistorizer::~QUaSqliteHistorizer()
{
	if (!QSqlDatabase::contains(m_strSqliteDbName))
	{
		return;
	}
	QSqlDatabase::database(m_strSqliteDbName).close();
}

QString QUaSqliteHistorizer::sqliteDbName() const
{
	return m_strSqliteDbName;
}

bool QUaSqliteHistorizer::setSqliteDbName(
	const QString& strSqliteDbName,
	QQueue<QUaLog>& logOut)
{
	// set internally
	m_strSqliteDbName = strSqliteDbName;
	// create and test open database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// close
	db.close();
	// success
	return true;
}

int QUaSqliteHistorizer::transactionTimeout() const
{
	return m_timeoutTransaction;
}

void QUaSqliteHistorizer::setTransactionTimeout(const int& timeoutMs)
{
	m_timeoutTransaction = (std::max)(0, timeoutMs);
}

bool QUaSqliteHistorizer::writeHistoryData(
	const QUaNodeId &nodeId,
	const QUaHistoryDataPoint& dataPoint,
	QQueue<QUaLog>& logOut
)
{
	// check if there are any queued logs that need to be reported
	if (!m_deferedLogOut.isEmpty())
	{
		logOut << m_deferedLogOut;
		m_deferedLogOut.clear();
	}
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// handle transactions
	if (!this->handleTransactions(db, logOut))
	{
		return false;
	}
	// check data table exists
	bool dataTableExists;
	if (!this->tableDataByNodeIdExists(db, nodeId, dataTableExists, logOut))
	{
		return false;
	}
	if (!dataTableExists)
	{
		// get sql data type to store
		auto dataType = QUaSqliteHistorizer::QVariantToQtType(dataPoint.value);
		if (!this->createDataNodeTable(db, nodeId, dataType, logOut))
		{
			return false;
		}
	}
	// insert new data point
	return this->insertDataPoint(
		db,
		nodeId,
		dataPoint,
		logOut
	);
}

bool QUaSqliteHistorizer::updateHistoryData(
	const QUaNodeId &nodeId,
	const QUaHistoryDataPoint& dataPoint,
	QQueue<QUaLog>& logOut
)
{
	Q_UNUSED(nodeId);
	Q_UNUSED(dataPoint);
	Q_UNUSED(logOut);
	// TODO : implement; left as exercise
	return false;
}

bool QUaSqliteHistorizer::removeHistoryData(
	const QUaNodeId &nodeId,
	const QDateTime& timeStart,
	const QDateTime& timeEnd,
	QQueue<QUaLog>& logOut
)
{
	Q_UNUSED(nodeId);
	Q_UNUSED(timeStart);
	Q_UNUSED(timeEnd);
	Q_UNUSED(logOut);
	// TODO : implement; left as exercise
	return false;
}

QDateTime QUaSqliteHistorizer::firstTimestamp(
	const QUaNodeId &nodeId,
	QQueue<QUaLog>& logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return QDateTime();
	}
	// check data table exists
	bool dataTableExists;
	if (!this->tableDataByNodeIdExists(db, nodeId, dataTableExists, logOut))
	{
		return QDateTime();
	}
	if (!dataTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying first timestamp. "
				"History database does not contain table for node id %1")
				.arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	// get prepared statement cache
	QSqlQuery& query = m_dataPrepStmts[nodeId].firstTimestamp;
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for first timestamp in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for first timestamp in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	// get time key
	QSqlRecord rec = query.record();
	int timeKeyCol = rec.indexOf("Time");
	Q_ASSERT(timeKeyCol >= 0);
	auto timeInt = query.value(timeKeyCol).toLongLong();
	return QDateTime::fromMSecsSinceEpoch(timeInt, Qt::UTC);
}

QDateTime QUaSqliteHistorizer::lastTimestamp(
	const QUaNodeId &nodeId,
	QQueue<QUaLog>& logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return QDateTime();
	}
	// check data table exists
	bool dataTableExists;
	if (!this->tableDataByNodeIdExists(db, nodeId, dataTableExists, logOut))
	{
		return QDateTime();
	}
	if (!dataTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying last timestamp. "
				"History database does not contain table for node id %1")
				.arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	// get prepared statement cache
	QSqlQuery& query = m_dataPrepStmts[nodeId].lastTimestamp;
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for last timestamp in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for last timestamp in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	// get time key
	QSqlRecord rec = query.record();
	int timeKeyCol = rec.indexOf("Time");
	Q_ASSERT(timeKeyCol >= 0);
	auto timeInt = query.value(timeKeyCol).toLongLong();
	return QDateTime::fromMSecsSinceEpoch(timeInt, Qt::UTC);
}

bool QUaSqliteHistorizer::hasTimestamp(
	const QUaNodeId &nodeId,
	const QDateTime& timestamp,
	QQueue<QUaLog>& logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// check data table exists
	bool dataTableExists;
	if (!this->tableDataByNodeIdExists(db, nodeId, dataTableExists, logOut))
	{
		return false;
	}
	if (!dataTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying exact timestamp. "
				"History database does not contain table for node id %1")
				.arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery& query = m_dataPrepStmts[nodeId].hasTimestamp;
	query.bindValue(0, timestamp.toMSecsSinceEpoch());
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for exact timestamp in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// exists if at least one result
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for exact timestamp in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Warning,
			QUaLogCategory::History
			});
		return false;
	}
	QSqlRecord rec = query.record();
	auto num = query.value(0).toULongLong();
	bool found = num > 0;
	return found;
}

QDateTime QUaSqliteHistorizer::findTimestamp(
	const QUaNodeId &nodeId,
	const QDateTime& timestamp,
	const QUaHistoryBackend::TimeMatch& match,
	QQueue<QUaLog>& logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return QDateTime();
	}
	// check data table exists
	bool dataTableExists;
	if (!this->tableDataByNodeIdExists(db, nodeId, dataTableExists, logOut))
	{
		return QDateTime();
	}
	if (!dataTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying around timestamp. "
				"History database does not contain table for node id %1")
				.arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	// get correct query
	QSqlQuery query;
	switch (match)
	{
	case QUaHistoryBackend::TimeMatch::ClosestFromAbove:
	{
		query = m_dataPrepStmts[nodeId].findTimestampAbove;
	}
	break;
	case QUaHistoryBackend::TimeMatch::ClosestFromBelow:
	{
		query = m_dataPrepStmts[nodeId].findTimestampBelow;
	}
	break;
	default:
	{
		Q_ASSERT(false);
	}
	break;
	}
	// set reference time
	query.bindValue(0, timestamp.toMSecsSinceEpoch());
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table around timestamp in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	// if there is none return either first or last
	if (!query.next())
	{
		switch (match)
		{
		case QUaHistoryBackend::TimeMatch::ClosestFromAbove:
		{
			return this->lastTimestamp(nodeId, logOut);
		}
		break;
		case QUaHistoryBackend::TimeMatch::ClosestFromBelow:
		{
			return this->firstTimestamp(nodeId, logOut);
		}
		break;
		default:
		{
			Q_ASSERT(false);
		}
		break;
		}
	}
	// get time key
	QSqlRecord rec = query.record();
	int timeKeyCol = rec.indexOf("Time");
	Q_ASSERT(timeKeyCol >= 0);
	auto timeInt = query.value(timeKeyCol).toLongLong();
	return QDateTime::fromMSecsSinceEpoch(timeInt, Qt::UTC);
}

quint64 QUaSqliteHistorizer::numDataPointsInRange(
	const QUaNodeId &nodeId,
	const QDateTime& timeStart,
	const QDateTime& timeEnd,
	QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return 0;
	}
	// check data table exists
	bool dataTableExists;
	if (!this->tableDataByNodeIdExists(db, nodeId, dataTableExists, logOut))
	{
		return 0;
	}
	if (!dataTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying number of points in range. "
				"History database does not contain table for node id %1")
				.arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return 0;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query;
	if (timeEnd.isValid())
	{
		query = m_dataPrepStmts[nodeId].numDataPointsInRangeEndValid;
		query.bindValue(0, timeStart.toMSecsSinceEpoch());
		query.bindValue(1, timeEnd.toMSecsSinceEpoch());
	}
	else
	{
		query = m_dataPrepStmts[nodeId].numDataPointsInRangeEndInvalid;
		query.bindValue(0, timeStart.toMSecsSinceEpoch());
	}
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for number of points in range in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return 0;
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for number of points in range in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Warning,
			QUaLogCategory::History
			});
		return 0;
	}
	// get number of points in range
	QSqlRecord rec = query.record();
	auto num = query.value(0).toULongLong();
	return num;
}

QVector<QUaHistoryDataPoint> QUaSqliteHistorizer::readHistoryData(
	const QUaNodeId &nodeId,
	const QDateTime& timeStart,
	const quint64& numPointsOffset,
	const quint64& numPointsToRead,
	QQueue<QUaLog>& logOut)
{
	auto points = QVector<QUaHistoryDataPoint>();
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return points;
	}
	// check data table exists
	bool dataTableExists;
	if (!this->tableDataByNodeIdExists(db, nodeId, dataTableExists, logOut))
	{
		return points;
	}
	if (!dataTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying data points. "
				"History database does not contain table for node id %1")
				.arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return points;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery& query = m_dataPrepStmts[nodeId].readHistoryData;
	query.bindValue(0, timeStart.toMSecsSinceEpoch());
	query.bindValue(1, numPointsToRead);
	query.bindValue(2, numPointsOffset);
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for data points in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return points;
	}
	while (query.next())
	{
		QSqlRecord rec = query.record();
		int timeKeyCol = rec.indexOf("Time");
		int valueKeyCol = rec.indexOf("Value");
		int statusKeyCol = rec.indexOf("Status");
		Q_ASSERT(timeKeyCol >= 0);
		Q_ASSERT(valueKeyCol >= 0);
		Q_ASSERT(statusKeyCol >= 0);
		auto timeInt = query.value(timeKeyCol).toLongLong();
		auto time = QDateTime::fromMSecsSinceEpoch(timeInt, Qt::UTC); // NOTE : expensive if spec not defined
		auto value = query.value(valueKeyCol);
		auto status = query.value(statusKeyCol).toUInt();
		points << QUaHistoryDataPoint({
			time, value, status
			});
	}
	// NOTE : return an invalid value if API requests more values than available
	while (points.count() < numPointsToRead)
	{
		points << QUaHistoryDataPoint();
	}
	return points;
}

// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

bool QUaSqliteHistorizer::writeHistoryEventsOfType(
	const QUaNodeId            &eventTypeNodeId,
	const QList<QUaNodeId>     &emittersNodeIds,
	const QUaHistoryEventPoint &eventPoint,
	QQueue<QUaLog>             &logOut
)
{
	// check if there are any queued logs that need to be reported
	if (!m_deferedLogOut.isEmpty())
	{
		logOut << m_deferedLogOut;
		m_deferedLogOut.clear();
	}
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// handle transactions
	if (!this->handleTransactions(db, logOut))
	{
		return false;
	}
	// check event data table exists
	bool eventTypeTableExists;
	if (!this->tableEventTypeByNodeIdExists(
		db, 
		eventTypeNodeId, 
		eventPoint, 
		eventTypeTableExists, 
		logOut))
	{
		return false;
	}
	if (!eventTypeTableExists)
	{
		// create event data table
		if (!this->createEventTypeTable(db, eventTypeNodeId, eventPoint, logOut))
		{
			return false;
		}
	}
	// check table of event type names exist
	bool eventTypeNameTableExists;
	if (!this->tableEventTypeNameExists(db, eventTypeNameTableExists, logOut))
	{
		return false;
	}
	if (!eventTypeNameTableExists)
	{
		// create event type name table
		if (!this->createEventTypeNameTable(db, logOut))
		{
			return false;
		}
	}
	// chech each emitter table exists
	for (auto &emitterNodeId : emittersNodeIds)
	{
		bool emitterTableExists;
		if (!this->tableEmitterByNodeIdExists(db, emitterNodeId, emitterTableExists, logOut))
		{
			return false;
		}
		if (!emitterTableExists)
		{
			// create emitter table
			if (!this->createEmitterTable(db, emitterNodeId, logOut))
			{
				return false;
			}
		}
	}
	// insert new event in its event type table, return new event key
	qint64 eventKey = -1;
	if (!this->insertEventPoint(db, eventTypeNodeId, eventPoint, eventKey, logOut))
	{
		return false;
	}
	// check if event type table name already in table else insert it, fetch event type name key
	qint64 outEventTypeKey = -1;
	if (!this->selectOrInsertEventTypeName(db, eventTypeNodeId, outEventTypeKey, logOut))
	{
		return false;
	}
	// insert reference to new event and event type in each emitter
	bool ok = true;
	for (auto& emitterNodeId : emittersNodeIds)
	{
		ok = ok && this->insertEventReferenceInEmitterTable(
			db,
			emitterNodeId,
			eventPoint.timestamp,
			outEventTypeKey,
			eventKey,
			logOut
		);
	}
	return ok;
}

QVector<QUaNodeId> QUaSqliteHistorizer::eventTypesOfEmitter(
	const QUaNodeId &emitterNodeId,
	QQueue<QUaLog>  &logOut
)
{
	QVector<QUaNodeId> retTypes;
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return retTypes;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	// check table of event type names exist
	bool eventTypeNameTableExists;
	if (!this->tableEventTypeNameExists(db, eventTypeNameTableExists, logOut))
	{
		return retTypes;
	}
	bool emitterTableExists;
	if (!this->tableEmitterByNodeIdExists(db, emitterNodeId, emitterTableExists, logOut))
	{
		return retTypes;
	}
	if (!eventTypeNameTableExists ||
		!emitterTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Could not fetch any event types for emitter [%1] in database %2.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName),
			QUaLogLevel::Warning,
			QUaLogCategory::History
			});
		return retTypes;
	}
	// create query
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT n.TableName FROM EventTypeTableNames n "
		"INNER JOIN "
			"("
			"SELECT DISTINCT EventType FROM \"%1\""
			") e "
		"ON n.EventTypeTableNames = e.EventType"
	).arg(emitterNodeId);
	// execute query
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not fetch any event types for emitter [%1] in database %2. Sql : %3.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return retTypes;
	}
	// loop results
	// NOTE : cannot prealloc retTypes size because QSqlQuery::size for sqlite returns -1
	while(query.next())
	{
		retTypes << query.value(0).toString();
	}
	return retTypes;
}

QDateTime QUaSqliteHistorizer::findTimestampEventOfType(
	const QUaNodeId                    &emitterNodeId,
	const QUaNodeId                    &eventTypeNodeId,
	const QDateTime                    &timestamp,
	const QUaHistoryBackend::TimeMatch &match,
	QQueue<QUaLog>                     &logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return QDateTime();
	}
	// check tables exist
	bool eventTypeTableExists;
	if (!this->tableEventTypeByNodeIdExists(
		db,
		eventTypeNodeId,
		QUaHistoryEventPoint(),
		eventTypeTableExists,
		logOut))
	{
		return QDateTime();
	}
	if (!eventTypeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying around timestamp. "
				"Event history database %1 does not contain table for event type [%2]")
				.arg(m_strSqliteDbName)
				.arg(eventTypeNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	bool emitterTableExists;
	if (!this->tableEmitterByNodeIdExists(db, emitterNodeId, emitterTableExists, logOut))
	{
		return QDateTime();
	}
	if (!emitterTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying around timestamp. "
				"Event history database %1 does not contain table for emitter [%2]")
				.arg(m_strSqliteDbName)
				.arg(emitterNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	// check if event type table name already in table else insert it, fetch event type name key
	qint64 outEventTypeKey = -1;
	if (!this->selectOrInsertEventTypeName(db, eventTypeNodeId, outEventTypeKey, logOut))
	{
		logOut << QUaLog({
			QObject::tr("Error querying around timestamp. "
				"Event history database %1 does not table name for event type [%2] in %3 table.")
				.arg(m_strSqliteDbName)
				.arg(eventTypeNodeId)
				.arg(QUaSqliteHistorizer::eventTypesTable),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	Q_ASSERT(outEventTypeKey >= 0);
	// get correct query
	QSqlQuery query(db);
	switch (match)
	{
	case QUaHistoryBackend::TimeMatch::ClosestFromAbove:
	{
		// prepared statement for find timestamp from above
		QString strStmt = QString(
			"SELECT "
				"e.Time "
			"FROM "
				"\"%1\" e "
			"WHERE "
				"e.Time >= :Time "
			"AND "
				"e.EventType = %2 "
			"ORDER BY "
				"e.Time ASC "
			"LIMIT "
				"1;"
		).arg(emitterNodeId).arg(outEventTypeKey);
		if (!this->prepareStmt(query, strStmt, logOut))
		{
			return QDateTime();
		}
	}
	break;
	case QUaHistoryBackend::TimeMatch::ClosestFromBelow:
	{
		// prepared statement for find timestamp from below
		QString strStmt = QString(
			"SELECT "
				"e.Time "
			"FROM "
				"\"%1\" e "
			"WHERE "
				"e.Time < :Time "
			"AND "
				"e.EventType = %2 "
			"ORDER BY "
				"e.Time DESC "
			"LIMIT "
				"1;"
		).arg(emitterNodeId).arg(outEventTypeKey);
		if (!this->prepareStmt(query, strStmt, logOut))
		{
			return QDateTime();
		}
	}
	break;
	default:
	{
		Q_ASSERT(false);
	}
	break;
	}
	// set reference time
	query.bindValue(0, timestamp.toMSecsSinceEpoch());
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] event emitter table around timestamp in %2 database. Sql : %3.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	// if there is none return either first or last
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Query [%1] event emitter table around timestamp returned empty in %2 database.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName),
			QUaLogLevel::Warning,
			QUaLogCategory::History
			});
		return QDateTime();
	}
	// get time key
	QSqlRecord rec = query.record();
	int timeKeyCol = rec.indexOf("Time");
	Q_ASSERT(timeKeyCol >= 0);
	auto timeInt = query.value(timeKeyCol).toLongLong();
	return QDateTime::fromMSecsSinceEpoch(timeInt, Qt::UTC);
}

quint64 QUaSqliteHistorizer::numEventsOfTypeInRange(
	const QUaNodeId &emitterNodeId,
	const QUaNodeId &eventTypeNodeId,
	const QDateTime &timeStart,
	const QDateTime &timeEnd,
	QQueue<QUaLog>  &logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return 0;
	}
	// check tables exist
	bool eventTypeTableExists;
	if (!this->tableEventTypeByNodeIdExists(
		db,
		eventTypeNodeId,
		QUaHistoryEventPoint(),
		eventTypeTableExists,
		logOut))
	{
		return 0;
	}
	if (!eventTypeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying number of events in time range. "
				"Event history database %1 does not contain table for event type [%2]")
				.arg(m_strSqliteDbName)
				.arg(eventTypeNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return 0;
	}
	bool emitterTableExists;
	if (!this->tableEmitterByNodeIdExists(db, emitterNodeId, emitterTableExists, logOut))
	{
		return 0;
	}
	if (!emitterTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying number of events in time range. "
				"Event history database %1 does not contain table for emitter [%2]")
				.arg(m_strSqliteDbName)
				.arg(emitterNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return 0;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	// check if event type table name already in table else insert it, fetch event type name key
	qint64 outEventTypeKey = -1;
	if (!this->selectOrInsertEventTypeName(db, eventTypeNodeId, outEventTypeKey, logOut))
	{
		logOut << QUaLog({
			QObject::tr("Error querying number of events in time range. "
				"Event history database %1 does not table name for event type [%2] in %3 table.")
				.arg(m_strSqliteDbName)
				.arg(eventTypeNodeId)
				.arg(QUaSqliteHistorizer::eventTypesTable),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return 0;
	}
	Q_ASSERT(outEventTypeKey >= 0);
	Q_ASSERT(timeStart.isValid() && timeEnd.isValid());
	QSqlQuery query(db);
	// prepared statement for num points in range
	QString strStmt = QString(
		"SELECT "
			"COUNT(*) "
		"FROM "
			"\"%1\" e "
		"WHERE "
			"e.Time >= :TimeStart "
		"AND "
			"e.Time <= :TimeEnd "
		"AND "
				"e.EventType = %2 "
		"ORDER BY "
			"e.Time ASC;"
	).arg(emitterNodeId).arg(outEventTypeKey);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	// bind
	query.bindValue(":TimeStart", timeStart.toMSecsSinceEpoch());
	query.bindValue(":TimeEnd"  , timeEnd.toMSecsSinceEpoch());
	// execute
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for number of events in range in %2 database. Sql : %3.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return 0;
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for events of events in range in %2 database. Sql : %3.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Warning,
			QUaLogCategory::History
			});
		return 0;
	}
	// get number of points in range
	QSqlRecord rec = query.record();
	auto num = query.value(0).toULongLong();
	return num;
}

QVector<QUaHistoryEventPoint> QUaSqliteHistorizer::readHistoryEventsOfType(
	const QUaNodeId &emitterNodeId,
	const QUaNodeId &eventTypeNodeId,
	const QDateTime &timeStart,
	const quint64   &numPointsOffset,
	const quint64   &numPointsToRead,
	QQueue<QUaLog>  &logOut
)
{
	auto points = QVector<QUaHistoryEventPoint>();
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return points;
	}
	// check tables exist
	bool eventTypeTableExists;
	if (!this->tableEventTypeByNodeIdExists(
		db,
		eventTypeNodeId,
		QUaHistoryEventPoint(),
		eventTypeTableExists,
		logOut))
	{
		return points;
	}
	if (!eventTypeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for event points. "
				"Event history database %2 does not contain table for event type [%1]")
				.arg(eventTypeNodeId)
				.arg(m_strSqliteDbName),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return points;
	}
	bool emitterTableExists;
	if (!this->tableEmitterByNodeIdExists(db, emitterNodeId, emitterTableExists, logOut))
	{
		return points;
	}
	if (!emitterTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for event points. "
				"Event history database %2 does not contain table for emitter [%1]")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return points;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	// check if event type table name already in table else insert it, fetch event type name key
	qint64 outEventTypeKey = -1;
	if (!this->selectOrInsertEventTypeName(db, eventTypeNodeId, outEventTypeKey, logOut))
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for event points. "
				"Event history database %2 does not table name for event type [%1] in %3 table.")
				.arg(eventTypeNodeId)
				.arg(m_strSqliteDbName)
				.arg(QUaSqliteHistorizer::eventTypesTable),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return points;
	}
	Q_ASSERT(outEventTypeKey >= 0);
	Q_ASSERT(timeStart.isValid());
	QSqlQuery query(db);
	// get column names
	QString strStmt = QString(
		"SELECT c.name FROM pragma_table_info(\"%1\") c;"
	).arg(eventTypeNodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return points;
	}
	// execute
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for column names in %2 database. Sql : %3.")
				.arg(eventTypeNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return points;
	}
	// read results
	QHash<QString, int> tableCols;
	while (query.next())
	{
		tableCols[query.value(0).toString()] = -1;
	}
	Q_ASSERT(tableCols.size() > 1);
	// prepared statement for num points in range
	strStmt = QString(
		"SELECT * FROM \"%1\" t "
		"INNER JOIN ("
			"SELECT "
				"EventId "
			"FROM "
				"\"%2\" e "
			"WHERE "
				"e.Time >= :TimeStart "
			"AND "
					"e.EventType = %3 "
			"ORDER BY "
				"e.Time ASC LIMIT :Limit OFFSET :Offset"
		") e "
		"ON t.\"%1\" = e.EventId;"
	).arg(eventTypeNodeId).arg(emitterNodeId).arg(outEventTypeKey);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return points;
	}
	// bind
	query.bindValue(":TimeStart", timeStart.toMSecsSinceEpoch());
	query.bindValue(":Limit"    , numPointsToRead);
	query.bindValue(":Offset"   , numPointsOffset);
	// execute
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for event points in %2 database. Sql : %3.")
				.arg(eventTypeNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return points;
	}
	// get column indexes
	auto i = tableCols.begin();
	while (i != tableCols.end())
	{
		auto& name  = i.key();
		auto& index = i.value();
		index = query.record().indexOf(name); // 0.17
		Q_ASSERT(index >= 0);
		++i;
	}
	// read results
	const QString strTimeColName("Time");
	while (query.next()) // 11.3%
	{
		qulonglong iTime = query.value(tableCols[strTimeColName]).toULongLong();
		QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(iTime, Qt::UTC); // NOTE : expensive if spec not defined
		// insert point
		points << QUaHistoryEventPoint({
			timestamp,
			QHash<QString, QVariant>()
		});
		int pointIndex = points.size() - 1;
		auto& fields = points[pointIndex].fields;
		// loop fields
		i = tableCols.begin();
		while (i != tableCols.end())
		{
			auto& name  = i.key();
			auto& index = i.value();
			fields.insert(name, query.value(index)); // 12%
			++i;
		}
	}
	return points;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

bool QUaSqliteHistorizer::getOpenedDatabase(
	QSqlDatabase& db,
	QQueue<QUaLog>& logOut
) const
{
	// add if not added
	if (QSqlDatabase::contains(m_strSqliteDbName))
	{
		db = QSqlDatabase::database(m_strSqliteDbName, true);
	}
	else
	{
		db = QSqlDatabase::addDatabase("QSQLITE", m_strSqliteDbName);
		// the database name is not the connection name
		db.setDatabaseName(m_strSqliteDbName);
		db.open();
	}
	// check if opened correctly
	if (!db.isOpen())
	{
		logOut << QUaLog({
			QObject::tr("Error opening %1. Sql : %2")
				.arg(m_strSqliteDbName)
				.arg(db.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	return true;
}

bool QUaSqliteHistorizer::tableExists(
	QSqlDatabase& db, 
	const QString& tableName, 
	bool& tableExists, 
	QQueue<QUaLog>& logOut
)
{
	// if not exists in cache, check database
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT "
			"name "
		"FROM "
			"sqlite_master "
		"WHERE "
			"type='table' "
		"AND "
			"name=\"%1\";"
	).arg(tableName);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying if table [%1] exists in %2 database. Sql : %3.")
				.arg(tableName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	// empty result if not exists
	tableExists = query.next();
	return true;
}

bool QUaSqliteHistorizer::tableDataByNodeIdExists(
	QSqlDatabase& db,
	const QUaNodeId &nodeId,
	bool& tableExists,
	QQueue<QUaLog>& logOut)
{
	// save time by using cache instead of SQL
	if (m_dataPrepStmts.contains(nodeId))
	{
		tableExists = true;
		return true;
	}
	// use generic method
	bool ok = this->tableExists(
		db,
		nodeId,
		tableExists,
		logOut
	);
	// early exit if cannot continue
	if (!ok || !tableExists)
	{
		return ok;
	}
	// cache prepared statement if table exists
	ok = this->dataPrepareAllStmts(db, nodeId, logOut);
	return ok;
}

bool QUaSqliteHistorizer::createDataNodeTable(
	QSqlDatabase& db,
	const QUaNodeId &nodeId,
	const QMetaType::Type& storeType,
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"CREATE TABLE \"%1\""
		"("
			"[%1] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
			"[Time] INTEGER NOT NULL,"
			"[Value] %2 NOT NULL,"
			"[Status] INTEGER NOT NULL"
		");"
	)
		.arg(nodeId)
		.arg(QUaSqliteHistorizer::QtTypeToSqlType(storeType));
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// create unique index on [Time] key for faster queries
	strStmt = QString("CREATE UNIQUE INDEX \"%1_Time\" ON \"%1\"(Time);").arg(nodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1_Time index on %1 table in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// cache prepared statement
	bool ok = this->dataPrepareAllStmts(db, nodeId, logOut);
	return ok;
}

bool QUaSqliteHistorizer::insertDataPoint(
	QSqlDatabase& db,
	const QUaNodeId &nodeId,
	const QUaHistoryDataPoint& dataPoint,
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	Q_ASSERT(m_dataPrepStmts.contains(nodeId));
	QSqlQuery& query = m_dataPrepStmts[nodeId].writeHistoryData;
	query.bindValue(0, dataPoint.timestamp.toMSecsSinceEpoch());
	query.bindValue(1, dataPoint.value);
	query.bindValue(2, dataPoint.status);
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Could not insert new row in %1 table in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	return true;
}

bool QUaSqliteHistorizer::dataPrepareAllStmts(
	QSqlDatabase& db,
	const QUaNodeId &nodeId,
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt;
	// prepared statement for insert
	strStmt = QString(
		"INSERT INTO \"%1\" (Time, Value, Status) VALUES (:Time, :Value, :Status);"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].writeHistoryData = query;
	// prepared statement for first timestamp
	strStmt = QString(
		"SELECT "
			"p.Time "
		"FROM "
			"\"%1\" p "
		"ORDER BY "
			"p.Time ASC "
		"LIMIT "
			"1;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].firstTimestamp = query;
	// prepared statement for last timestamp
	strStmt = QString(
		"SELECT "
			"p.Time "
		"FROM "
			"\"%1\" p "
		"ORDER BY "
			"p.Time DESC "
		"LIMIT "
			"1;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].lastTimestamp = query;
	// prepared statement for has timestamp
	strStmt = QString(
		"SELECT "
			"COUNT(*) "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time = :Time;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].hasTimestamp = query;
	// prepared statement for find timestamp from above
	strStmt = QString(
		"SELECT "
			"p.Time "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time > :Time "
		"ORDER BY "
			"p.Time ASC "
		"LIMIT "
			"1;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].findTimestampAbove = query;
	// prepared statement for find timestamp from below
	strStmt = QString(
		"SELECT "
			"p.Time "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time < :Time "
		"ORDER BY "
			"p.Time DESC "
		"LIMIT "
			"1;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].findTimestampBelow = query;
	// prepared statement for num points in range when end time is valid
	strStmt = QString(
		"SELECT "
			"COUNT(*) "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time >= :TimeStart "
		"AND "
			"p.Time <= :TimeEnd "
		"ORDER BY "
			"p.Time ASC;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].numDataPointsInRangeEndValid = query;
	// prepared statement for num points in range when end time is invalid
	strStmt = QString(
		"SELECT "
			"COUNT(*) "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time >= :TimeStart "
		"ORDER BY "
			"p.Time ASC;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].numDataPointsInRangeEndInvalid = query;
	// prepared statement for reading data points
	strStmt = QString(
		"SELECT "
			"p.Time, p.Value, p.Status "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time >= :Time "
		"ORDER BY "
			"p.Time ASC "
		"LIMIT "
			":Limit "
		"OFFSET "
			":Offset;"
	).arg(nodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_dataPrepStmts[nodeId].readHistoryData = query;
	// success
	return true;
}

bool QUaSqliteHistorizer::prepareStmt(
	QSqlQuery& query, 
	const QString& strStmt,
	QQueue<QUaLog>& logOut)
{
	if (!query.prepare(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error preparing statement %1 for %2 database. Sql : %3.")
				.arg(strStmt)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	return true;
}

bool QUaSqliteHistorizer::handleTransactions(
	QSqlDatabase& db, 
	QQueue<QUaLog>& logOut)
{
	// return success if transactions disabled
	if (m_timeoutTransaction == 0)
	{
		return true;
	}
	// return success if transaction currently opened
	if (m_timerTransaction.isActive())
	{
		return true;
	}
	// open new transaction if required
	if (!db.transaction())
	{
		logOut << QUaLog({
			QObject::tr("Failed to begin transaction in %1 database. Sql : %2.")
				.arg(m_strSqliteDbName)
				.arg(db.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	// start timer to stop transaction after specified period (see constructor)
	m_timerTransaction.start(m_timeoutTransaction);
	return true;
}

QMetaType::Type QUaSqliteHistorizer::QVariantToQtType(const QVariant& value)
{
	return static_cast<QMetaType::Type>(
		value.type() < 1024 ?
		value.type() :
		value.userType()
	);
}

const QString QUaSqliteHistorizer::QtTypeToSqlType(const QMetaType::Type& qtType)
{

	if (!QUaSqliteHistorizer::m_hashTypes.contains(qtType))
	{
		qWarning() << "[UNKNOWN TYPE]" << QMetaType::typeName(qtType);
		Q_ASSERT_X(false, "QUaSqliteHistorizer::QtTypeToSqlType", "Unknown type.");
	}
	return QUaSqliteHistorizer::m_hashTypes.value(qtType, "BLOB");
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

bool QUaSqliteHistorizer::tableEventTypeByNodeIdExists(
	QSqlDatabase& db, 
	const QUaNodeId& eventTypeNodeId, 
	const QUaHistoryEventPoint& eventPoint,
	bool& tableExists, 
	QQueue<QUaLog>& logOut)
{
	// save time by using cache instead of SQL
	if (m_eventTypePrepStmts.contains(eventTypeNodeId))
	{
		tableExists = true;
		return true;
	}
	// use generic method
	bool ok = this->tableExists(
		db,
		eventTypeNodeId,
		tableExists,
		logOut
	);
	// early exit if cannot continue
	if (!ok || !tableExists)
	{
		return ok;
	}
	// cache prepared statement if table exists
	ok = this->eventTypePrepareStmt(db, eventTypeNodeId, eventPoint, logOut);
	return ok;
}

bool QUaSqliteHistorizer::createEventTypeTable(
	QSqlDatabase& db, 
	const QUaNodeId& eventTypeNodeId, 
	const QUaHistoryEventPoint& eventPoint, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"CREATE TABLE \"%1\""
		"("
			"[%1] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
	).arg(eventTypeNodeId);
	// add columns
	QHashIterator<QString, QVariant> i(eventPoint.fields);
	while (i.hasNext()) {
		i.next();
		auto& name = i.key();
		auto& value = i.value();
		strStmt += QString("[%1] %2")
			.arg(name)
			.arg(
				QUaSqliteHistorizer::QtTypeToSqlType(QUaSqliteHistorizer::QVariantToQtType(value)));
		if (i.hasNext())
		{
			strStmt += ", ";
		}
	}
	// close statement
	strStmt += ");";
	// execute
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1 event type table in %2 database. Sql : %3.")
				.arg(eventTypeNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// cache prepared statement if table exists
	bool ok = this->eventTypePrepareStmt(db, eventTypeNodeId, eventPoint, logOut);
	return ok;
}

bool QUaSqliteHistorizer::eventTypePrepareStmt(
	QSqlDatabase& db, 
	const QUaNodeId& eventTypeNodeId, 
	const QUaHistoryEventPoint& eventPoint, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt;
	// prepared statement for insert
	strStmt = QString(
		"INSERT INTO \"%1\" ("
	).arg(eventTypeNodeId);
	// iterate column names (e.g. Time, Value, Status)
	QHashIterator<QString, QVariant> i(eventPoint.fields);
	while (i.hasNext()) {
		i.next();
		auto& name = i.key();
		strStmt += QString("%1")
			.arg(name);
		if (i.hasNext())
		{
			strStmt += ", ";
		}
	}
	// continue statement
	strStmt += ") VALUES (";
	// iterate column placeholders (e.g. :Time, :Value, :Status)
	i.toFront();
	while (i.hasNext()) {
		i.next();
		auto& name = i.key();
		strStmt += QString(":%1")
			.arg(name);
		if (i.hasNext())
		{
			strStmt += ", ";
		}
	}
	// close statement
	strStmt += ");";
	// prepare
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_eventTypePrepStmts[eventTypeNodeId] = query;
	return true;
}

QString QUaSqliteHistorizer::eventTypesTable = "EventTypeTableNames";

bool QUaSqliteHistorizer::tableEventTypeNameExists(
	QSqlDatabase& db, 
	bool& tableExists, 
	QQueue<QUaLog>& logOut)
{
	// save time by using cache instead of SQL
	if (m_eventTypeNamePrepStmt.contains(QUaSqliteHistorizer::eventTypesTable))
	{
		tableExists = true;
		return true;
	}
	// use generic method
	bool ok = this->tableExists(
		db,
		QUaSqliteHistorizer::eventTypesTable,
		tableExists,
		logOut
	);
	// early exit if cannot continue
	if (!ok || !tableExists)
	{
		return ok;
	}
	// cache prepared statement if table exists
	ok = this->eventTypeNamePrepareStmt(db, logOut);
	return ok;
}

bool QUaSqliteHistorizer::createEventTypeNameTable(
	QSqlDatabase& db, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"CREATE TABLE \"%1\""
		"("
			"[%1] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
			"[TableName] TEXT NOT NULL"
		");"
	).arg(QUaSqliteHistorizer::eventTypesTable);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg(QUaSqliteHistorizer::eventTypesTable)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// create unique index on [TableName] key for faster queries
	strStmt = QString("CREATE UNIQUE INDEX \"%1_TableName\" ON \"%1\"(TableName);")
		.arg(QUaSqliteHistorizer::eventTypesTable);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1_TableName index on %1 table in %2 database. Sql : %3.")
				.arg(QUaSqliteHistorizer::eventTypesTable)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// cache prepared statement
	bool ok = this->eventTypeNamePrepareStmt(db, logOut);
	return ok;
}

bool QUaSqliteHistorizer::eventTypeNamePrepareStmt(
	QSqlDatabase& db, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt;
	// prepared statement for insert
	strStmt = QString(
		"INSERT INTO \"%1\" (TableName) VALUES (:TableName);"
	).arg(QUaSqliteHistorizer::eventTypesTable);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_eventTypeNamePrepStmt[QUaSqliteHistorizer::eventTypesTable].insertEventTypeName = query;
	// prepared statement select exsiting
	strStmt = QString(
		"SELECT "
			"t.%1 "
		"FROM "
			"%1 t "
		"WHERE "
			"t.TableName = :TableName"
	).arg(QUaSqliteHistorizer::eventTypesTable);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_eventTypeNamePrepStmt[QUaSqliteHistorizer::eventTypesTable].selectEventTypeName = query;
	return true;
}

bool QUaSqliteHistorizer::tableEmitterByNodeIdExists(
	QSqlDatabase& db, const 
	QUaNodeId& emitterNodeId, 
	bool& tableExists, 
	QQueue<QUaLog>& logOut)
{
	// save time by using cache instead of SQL
	if (m_emitterPrepStmts.contains(emitterNodeId))
	{
		tableExists = true;
		return true;
	}
	// use generic method
	bool ok = this->tableExists(
		db,
		emitterNodeId,
		tableExists,
		logOut
	);
	// early exit if cannot continue
	if (!ok || !tableExists)
	{
		return ok;
	}
	// cache prepared statement if table exists
	ok = this->emitterPrepareStmt(db, emitterNodeId, logOut);
	return ok;
}

bool QUaSqliteHistorizer::createEmitterTable(
	QSqlDatabase& db, 
	const QUaNodeId& emitterNodeId, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"CREATE TABLE \"%1\""
		"("
			"[%1] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
			"[Time] INTEGER NOT NULL, "
			"[EventType] INTEGER NOT NULL, "
			"[EventId] INTEGER NOT NULL"
		");"
	).arg(emitterNodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// create index on [Time, EventType] key for faster queries
	strStmt = QString("CREATE INDEX \"%1_Time_EventType\" ON \"%1\"(Time, EventType);")
		.arg(emitterNodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1_Time_EventType on %1 table in %2 database. Sql : %3.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// cache prepared statement
	bool ok = this->emitterPrepareStmt(db, emitterNodeId, logOut);
	return ok;
}

bool QUaSqliteHistorizer::emitterPrepareStmt(
	QSqlDatabase& db, 
	const QUaNodeId& emitterNodeId, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt;
	// prepared statement for insert
	strStmt = QString(
		"INSERT INTO \"%1\" (Time, EventType, EventId) VALUES (:Time, :EventType, :EventId);"
	).arg(emitterNodeId);
	if (!this->prepareStmt(query, strStmt, logOut))
	{
		return false;
	}
	m_emitterPrepStmts[emitterNodeId] = query;
	return true;
}

bool QUaSqliteHistorizer::insertEventPoint(
	QSqlDatabase& db, 
	const QUaNodeId& eventTypeNodeId, 
	const QUaHistoryEventPoint& eventPoint, 
	qint64& outEventKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	Q_ASSERT(m_eventTypePrepStmts.contains(eventTypeNodeId));
	QSqlQuery& query = m_eventTypePrepStmts[eventTypeNodeId];
	// iterate column placeholders (e.g. Time, Value, Status)
	// and bind values
	QHashIterator<QString, QVariant> i(eventPoint.fields);
	while (i.hasNext()) {
		i.next();
		auto& name = i.key();
		auto& value = i.value();
		auto type = QUaSqliteHistorizer::QVariantToQtType(value);
		query.bindValue(
			QString(":%1").arg(name),
			type == QMetaType::UChar ? 
				value.toUInt() : 
			type == QMetaType::QDateTime ?
				value.toDateTime().toMSecsSinceEpoch() : 
				value
		);
	}
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Could not insert new row in %1 table in %2 database. Sql : %3.")
				.arg(eventTypeNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// get new key
	outEventKey = static_cast<qint64>(query.lastInsertId().toULongLong());
	return true;
}

bool QUaSqliteHistorizer::selectOrInsertEventTypeName(
	QSqlDatabase& db, 
	const QUaNodeId& eventTypeNodeId, 
	qint64& outEventTypeKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	Q_ASSERT(m_eventTypeNamePrepStmt.contains(QUaSqliteHistorizer::eventTypesTable));
	// first try to find with select
	QSqlQuery& querySelect = m_eventTypeNamePrepStmt[QUaSqliteHistorizer::eventTypesTable].selectEventTypeName;
	//  bind values
	querySelect.bindValue(0, eventTypeNodeId.toXmlString());
	// execute
	if (!querySelect.exec())
	{
		logOut << QUaLog({
			QObject::tr("Could not select row in %1 table in %2 database. Sql : %3.")
				.arg(QUaSqliteHistorizer::eventTypesTable)
				.arg(m_strSqliteDbName)
				.arg(querySelect.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// check if exists
	if(querySelect.next())
	{
		outEventTypeKey = static_cast<qint64>(querySelect.value(0).toLongLong());
		return true;
	}
	// insert
	QSqlQuery& queryInsert = m_eventTypeNamePrepStmt[QUaSqliteHistorizer::eventTypesTable].insertEventTypeName;
	//  bind values
	queryInsert.bindValue(0, eventTypeNodeId.toXmlString());
	// execute
	if (!queryInsert.exec())
	{
		logOut << QUaLog({
			QObject::tr("Could not insert new row in %1 table in %2 database. Sql : %3.")
				.arg(QUaSqliteHistorizer::eventTypesTable)
				.arg(m_strSqliteDbName)
				.arg(queryInsert.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	// get new key
	outEventTypeKey = static_cast<qint64>(queryInsert.lastInsertId().toULongLong());
	return true;
}

bool QUaSqliteHistorizer::insertEventReferenceInEmitterTable(
	QSqlDatabase& db, 
	const QUaNodeId& emitterNodeId, 
	const QDateTime& timestamp, 
	qint64& outEventTypeKey, 
	qint64& outEventKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	Q_ASSERT(m_emitterPrepStmts.contains(emitterNodeId));
	QSqlQuery& query = m_emitterPrepStmts[emitterNodeId];
	//  bind values
	query.bindValue(":Time"     , timestamp.toMSecsSinceEpoch());
	query.bindValue(":EventType", outEventTypeKey);
	query.bindValue(":EventId"  , outEventKey);
	// execute
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Could not insert new row in %1 table in %2 database. Sql : %3.")
				.arg(emitterNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
			});
		return false;
	}
	return true;
}


#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // UA_ENABLE_HISTORIZING