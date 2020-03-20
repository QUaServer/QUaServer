#include "quasqlitehistorizer.h"

#include <QUaServer>
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
	{qMetaTypeId<QUaDataType>(), "INTEGER"},
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
	{QMetaType::UnknownType    , "BLOB"   }
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
	const QString &strNodeId,
	const QUaHistoryBackend::DataPoint &dataPoint,
	QQueue<QUaLog> &logOut
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
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, strNodeId, typeTableExists, logOut))
	{
		return false;
	}
	if (!typeTableExists)
	{
		// get sql data type to store
		auto dataType = static_cast<QMetaType::Type>(
			dataPoint.value.type() < 1024 ?
			dataPoint.value.type() :
			dataPoint.value.userType()
		);
		if (!this->createNodeTable(db, strNodeId, dataType, logOut))
		{
			return false;
		}
	}
	// insert new data point
	return this->insertNewDataPoint(
		db,
		strNodeId,
		dataPoint,
		logOut
	);
}

bool QUaSqliteHistorizer::updateHistoryData(
	const QString   &strNodeId, 
	const QUaHistoryBackend::DataPoint &dataPoint,
	QQueue<QUaLog> &logOut
)
{
	// TODO :
	return false;
}

bool QUaSqliteHistorizer::removeHistoryData(
	const QString   &strNodeId,
	const QDateTime &timeStart,
	const QDateTime &timeEnd,
	QQueue<QUaLog>  &logOut
)
{
	// TODO :
	return false;
}

QDateTime QUaSqliteHistorizer::firstTimestamp(
	const QString  &strNodeId,
	QQueue<QUaLog> &logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return QDateTime();
	}
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, strNodeId, typeTableExists, logOut))
	{
		return QDateTime();
	}
	if (!typeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying first timestamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return QDateTime();
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT "
			"p.Time "
		"FROM "
			"\"%1\" p "
		"ORDER BY "
			"p.Time ASC "
		"LIMIT "
			"1;"
	).arg(strNodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for first timestamp in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return QDateTime();
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for first timestamp in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return QDateTime();
	}
	// get time key
	QSqlRecord rec = query.record();
	int timeKeyCol = rec.indexOf("Time");
	Q_ASSERT(timeKeyCol >= 0);
	auto timeInt = query.value(timeKeyCol).toLongLong();
	return QDateTime::fromMSecsSinceEpoch(timeInt);
}

QDateTime QUaSqliteHistorizer::lastTimestamp(
	const QString  &strNodeId,
	QQueue<QUaLog> &logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return QDateTime();
	}
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, strNodeId, typeTableExists, logOut))
	{
		return QDateTime();
	}
	if (!typeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying last timestamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return QDateTime();
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT "
			"p.Time "
		"FROM "
			"\"%1\" p "
		"ORDER BY "
			"p.Time DESC "
		"LIMIT "
			"1;"
	).arg(strNodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for last timestamp in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return QDateTime();
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for last timestamp in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return QDateTime();
	}
	// get time key
	QSqlRecord rec = query.record();
	int timeKeyCol = rec.indexOf("Time");
	Q_ASSERT(timeKeyCol >= 0);
	auto timeInt = query.value(timeKeyCol).toLongLong();
	return QDateTime::fromMSecsSinceEpoch(timeInt);
}

bool QUaSqliteHistorizer::hasTimestamp(
	const QString   &strNodeId,
	const QDateTime &timestamp,
	QQueue<QUaLog>  &logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, strNodeId, typeTableExists, logOut))
	{
		return false;
	}
	if (!typeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying exact timestamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT "
			"COUNT(*) "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time = %2;"
	).arg(strNodeId).arg(timestamp.toMSecsSinceEpoch());
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for exact timestamp in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// exists if at least one result
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for exact timestamp in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		});
		return false;
	}
	QSqlRecord rec = query.record();
	auto num   = query.value(0).toULongLong();
	bool found = num > 0;
	return found;
}

QDateTime QUaSqliteHistorizer::findTimestamp(
	const QString   &strNodeId,
	const QDateTime &timestamp,
	const QUaHistoryBackend::TimeMatch& match,
	QQueue<QUaLog>  &logOut
)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return QDateTime();
	}
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, strNodeId, typeTableExists, logOut))
	{
		return QDateTime();
	}
	if (!typeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying around timestamp. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return QDateTime();
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt;
	switch (match)
	{
		case QUaHistoryBackend::TimeMatch::ClosestFromAbove:
		{
			strStmt = QString(
				"SELECT "
					"p.Time "
				"FROM "
					"\"%1\" p "
				"WHERE "
					"p.Time > %2 "
				"ORDER BY "
					"p.Time ASC "
				"LIMIT "
					"1;"
			).arg(strNodeId).arg(timestamp.toMSecsSinceEpoch());
		}
		break;
		case QUaHistoryBackend::TimeMatch::ClosestFromBelow:
		{
			strStmt = QString(
				"SELECT "
					"p.Time "
				"FROM "
					"\"%1\" p "
				"WHERE "
					"p.Time < %2 "
				"ORDER BY "
					"p.Time DESC "
				"LIMIT "
					"1;"
			).arg(strNodeId).arg(timestamp.toMSecsSinceEpoch());
		}
		break;
		default:
		{
			Q_ASSERT(false);
		}
		break;
	}
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table around timestamp in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
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
				return this->lastTimestamp(strNodeId, logOut);
			}
			break;
			case QUaHistoryBackend::TimeMatch::ClosestFromBelow:
			{
				return this->firstTimestamp(strNodeId, logOut);
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
	return QDateTime::fromMSecsSinceEpoch(timeInt);
}

quint64 QUaSqliteHistorizer::numDataPointsInRange(
	const QString   &strNodeId,
	const QDateTime &timeStart,
	const QDateTime &timeEnd,
	QQueue<QUaLog>  &logOut)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return 0;
	}
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, strNodeId, typeTableExists, logOut))
	{
		return 0;
	}
	if (!typeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying number of points in range. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return 0;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt;
	if (timeEnd.isValid())
	{
		strStmt = QString(
			"SELECT "
				"COUNT(*) "
			"FROM "
				"\"%1\" p "
			"WHERE "
				"p.Time >= %2 "
			"AND "
				"p.Time <= %3 "
			"ORDER BY "
				"p.Time ASC;"
		).arg(strNodeId).arg(timeStart.toMSecsSinceEpoch()).arg(timeEnd.toMSecsSinceEpoch());
	}
	else
	{
		strStmt = QString(
			"SELECT "
				"COUNT(*) "
			"FROM "
				"\"%1\" p "
			"WHERE "
				"p.Time >= %2 "
			"ORDER BY "
				"p.Time ASC;"
		).arg(strNodeId).arg(timeStart.toMSecsSinceEpoch());
	}
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for number of points in range in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return 0;
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Empty result querying [%1] table for number of points in range in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Warning,
			QUaLogCategory::Serialization
		});
		return 0;
	}
	// get number of points in range
	QSqlRecord rec = query.record();
	auto num = query.value(0).toULongLong();
	return num;
}

QVector<QUaHistoryBackend::DataPoint> QUaSqliteHistorizer::readHistoryData(
	const QString   &strNodeId,
	const QDateTime &timeStart,
	const quint64   &numPointsToRead,
	QQueue<QUaLog>  &logOut)
{
	auto points = QVector<QUaHistoryBackend::DataPoint>();
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return points;
	}
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, strNodeId, typeTableExists, logOut))
	{
		return points;
	}
	if (!typeTableExists)
	{
		logOut << QUaLog({
			QObject::tr("Error querying data points. "
				"History database does not contain table for node id %1")
				.arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return points;
	}
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT "
			"p.Time, p.Value, p.Status "
		"FROM "
			"\"%1\" p "
		"WHERE "
			"p.Time >= %2 "
		"ORDER BY "
			"p.Time ASC "
		"LIMIT "
			"%3;"
	).arg(strNodeId).arg(timeStart.toMSecsSinceEpoch()).arg(numPointsToRead);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying [%1] table for data points in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return points;
	}
	while (query.next())
	{
		QSqlRecord rec   = query.record();
		int timeKeyCol   = rec.indexOf("Time");
		int valueKeyCol  = rec.indexOf("Value");
		int statusKeyCol = rec.indexOf("Status");
		Q_ASSERT(timeKeyCol   >= 0);
		Q_ASSERT(valueKeyCol  >= 0);
		Q_ASSERT(statusKeyCol >= 0);
		auto timeInt = query.value(timeKeyCol).toLongLong();
		auto time    = QDateTime::fromMSecsSinceEpoch(timeInt);
		auto value   = query.value(valueKeyCol);
		auto status  = query.value(statusKeyCol).toUInt();
		points << QUaHistoryBackend::DataPoint({
			time, value, status
		});
	}
	// NOTE : return an invalid value if API requests more values than available
	while (points.count() < numPointsToRead)
	{
		points << QUaHistoryBackend::DataPoint();
	}
	return points;
}

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
	const QString& strNodeId,
	bool& tableExists,
	QQueue<QUaLog>& logOut)
{
	// save time by using cache instead of SQL
	if (m_prepInsertStmts.contains(strNodeId))
	{
		tableExists = true;
		return true;
	}
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
	).arg(strNodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error querying if table [%1] exists in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// empty result if not exists
	if (!query.next())
	{
		tableExists = false;
		return true;
	}
	tableExists = true;
	// cache prepared statement if table exists
	if (!this->prepareInsertStmt(strNodeId, query, logOut))
	{
		return false;
	}
	m_prepInsertStmts[strNodeId] = query;
	return true;
}

bool QUaSqliteHistorizer::createNodeTable(
	QSqlDatabase& db, 
	const QString& strNodeId, 
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
	.arg(strNodeId)
	.arg(QUaSqliteHistorizer::QtTypeToSqlType(
			static_cast<QMetaType::Type>(storeType)
		)
	);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	// create unique index on [Time] key for faster queries
	strStmt = QString("CREATE UNIQUE INDEX \"%1_Time\" ON \"%1\"(Time);").arg(strNodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1_Time index on %1 table in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	// cache prepared statement
	if (!this->prepareInsertStmt(strNodeId, query, logOut))
	{
		return false;
	}
	m_prepInsertStmts[strNodeId] = query;
	return true;
}

bool QUaSqliteHistorizer::insertNewDataPoint(
	QSqlDatabase& db, 
	const QString& strNodeId, 
	const QUaHistoryBackend::DataPoint& dataPoint, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	Q_ASSERT(m_prepInsertStmts.contains(strNodeId));
	QSqlQuery& query = m_prepInsertStmts[strNodeId];
	query.bindValue(0, dataPoint.timestamp.toMSecsSinceEpoch());
	query.bindValue(1, dataPoint.value);
	query.bindValue(2, dataPoint.status);
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Could not insert new row in %1 table in %2 database. Sql : %3.")
				.arg(strNodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	return true;
}

bool QUaSqliteHistorizer::prepareInsertStmt(
	const QString& strNodeId, 
	QSqlQuery& query,
	QQueue<QUaLog>& logOut) const
{
	// create prepared statement for insert
	auto strStmt = QString(
		"INSERT INTO \"%1\" (Time, Value, Status) VALUES (:Time, :Value, :Status);"
	).arg(strNodeId);
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

const QString QUaSqliteHistorizer::QtTypeToSqlType(const QMetaType::Type& qtType)
{
	Q_ASSERT_X(
		QUaSqliteHistorizer::m_hashTypes.contains(qtType),
		"QtTypeToSqlType", "Unknown type."
	);
	return QUaSqliteHistorizer::m_hashTypes.value(qtType, "BLOB");
}