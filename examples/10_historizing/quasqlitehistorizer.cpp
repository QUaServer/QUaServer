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

bool QUaSqliteHistorizer::getOpenedDatabase(
	QSqlDatabase& db,
	QQueue<QUaLog>& logOut
)
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
	const QString& strTableName,
	bool& tableExists,
	QQueue<QUaLog>& logOut)
{
	Q_UNUSED(logOut);
	// save time by using cache instead of SQL
	Q_UNUSED(db);
	tableExists = m_prepStmts.contains(strTableName);
	return true;
}


bool QUaSqliteHistorizer::createNodeTable(
	QSqlDatabase& db, 
	const QString& strTableName, 
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
	.arg(strTableName)
	.arg(QUaSqliteHistorizer::QtTypeToSqlType(
			static_cast<QMetaType::Type>(storeType)
		)
	);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg(strTableName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	// create unique index on [Time] key for faster queries
	strStmt = QString("CREATE UNIQUE INDEX \"%1_Time\" ON \"%1\"(Time);").arg(strTableName);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Could not create %1_Time index on %1 table in %2 database. Sql : %3.")
				.arg(strTableName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::History
		});
		return false;
	}
	// create prepared statement for insert
	strStmt = QString(
		"INSERT INTO \"%1\" (Time, Value, Status) VALUES (:Time, :Value, :Status);"
	).arg(strTableName);
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
	// cache prepared statement
	m_prepStmts[strTableName] = query;
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