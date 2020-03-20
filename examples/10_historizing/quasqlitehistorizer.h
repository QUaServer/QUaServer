#ifndef QUASQLITEHISTORIZER_H
#define QUASQLITEHISTORIZER_H

#include <QUaHistoryBackend>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>

class QUaSqliteHistorizer
{
public:
	QUaSqliteHistorizer();
	~QUaSqliteHistorizer();

	// set sqlite database file to read from or write to
	QString sqliteDbName() const;
	bool setSqliteDbName(
		const QString& strSqliteDbName,
		QQueue<QUaLog>& logOut
	);

	// time period between opening and commiting a database transaction
	// this is implemented for performance reasons
	// default is 1000ms, a value <= 0 disables the use of transactions
	int transactionTimeout() const;
	void setTransactionTimeout(const int &timeoutMs);
	
	// required API for QUaServer::setHistorizer
	// write data point to backend, return true on success
	bool writeHistoryData(
		const QString &strNodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// update an existing node's data point in backend, return true on success
	bool updateHistoryData(
		const QString& strNodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// remove an existing node's data points within a range, return true on success
	bool removeHistoryData(
		const QString& strNodeId,
		const QDateTime& timeStart,
		const QDateTime& timeEnd,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the timestamp of the first sample available for the given node
	QDateTime firstTimestamp(
		const QString& strNodeId,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the timestamp of the latest sample available for the given node
	QDateTime lastTimestamp(
		const QString& strNodeId,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return true if given timestamp is available for the given node
	bool hasTimestamp(
		const QString& strNodeId,
		const QDateTime& timestamp,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return a timestamp matching the criteria for the given node
	QDateTime findTimestamp(
		const QString& strNodeId,
		const QDateTime& timestamp,
		const QUaHistoryBackend::TimeMatch& match,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the number for data points within a time range for the given node
	quint64 numDataPointsInRange(
		const QString& strNodeId,
		const QDateTime& timeStart,
		const QDateTime& timeEnd,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the numPointsToRead data points for the given node form the given start time
	QVector<QUaHistoryDataPoint> readHistoryData(
		const QString& strNodeId,
		const QDateTime& timeStart,
		const quint64& numPointsToRead,
		QQueue<QUaLog>& logOut
	);

private:
	QString m_strSqliteDbName;
	QTimer m_timerTransaction;
	int    m_timeoutTransaction;
	QQueue<QUaLog> m_deferedLogOut;
	// get database handle, creates it if not already
	bool getOpenedDatabase(
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	) const;
	// check table exists
	bool tableExists(
		QSqlDatabase& db,
		const QString& strNodeId,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create node history table
	bool createNodeTable(
		QSqlDatabase& db,
		const QString& strNodeId,
		const QMetaType::Type& storeType,
		QQueue<QUaLog>& logOut
	);
	// insert new data point into node history table
	bool insertNewDataPoint(
		QSqlDatabase& db,
		const QString& strNodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// check if transaction is needed
	bool handleTransactions(
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// prepared statements cache
	struct PreparedStatements {
		QSqlQuery writeHistoryData;
		QSqlQuery firstTimestamp;
		QSqlQuery lastTimestamp;
		QSqlQuery hasTimestamp;
		QSqlQuery findTimestampAbove;
		QSqlQuery findTimestampBelow;
		QSqlQuery numDataPointsInRangeEndValid;
		QSqlQuery numDataPointsInRangeEndInvalid;
		QSqlQuery readHistoryData;
	};
	QHash<QString, PreparedStatements> m_prepStmts;
	// prepare statement to insert history data points
	bool prepareAllStmts(
		QSqlDatabase& db,
		const QString& strNodeId,
		QQueue<QUaLog>& logOut
	);
	bool prepareStmt(
		QSqlQuery &query,
		const QString &strStmt,
		QQueue<QUaLog>& logOut
	);

	// return SQL type in string form, for given Qt type (only QUaServer supported types)
	static QHash<int, QString> m_hashTypes;
	static const QString QtTypeToSqlType(const QMetaType::Type& qtType);
};


#endif // QUASQLITEHISTORIZER_H
