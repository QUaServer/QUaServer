#ifndef QUAMULTISQLITEHISTORIZER_H
#define QUAMULTISQLITEHISTORIZER_H

#ifdef UA_ENABLE_HISTORIZING

#include <QUaHistoryBackend>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QElapsedTimer>

class QUaMultiSqliteHistorizer
{
public:
	QUaMultiSqliteHistorizer();
	~QUaMultiSqliteHistorizer();

	// path of directory where database files will be saved in
	// default is "." (current directory)
	QString databasePath() const;
	bool setDatabasePath(
		const QString& databasePath,
		QQueue<QUaLog>& logOut
	);

	// maximum size in megabytes to be used by a single database file
	// default value is 1Mb, a value <= 0 disables database size control (including max size control)
	double fileSizeLimMb() const;
	void setFileSizeLimMb(const double &fileSizeLimMb);

	// maximum size in megabytes to be used by the sum of all database files
	// default value is 50Mb, a value <= 0 disables maximum database size control
	// if != 0, then this value is limited in its lower bound by the fileSizeLimMb
	double totalSizeLimMb() const;
	void setTotalSizeLimMb(const double &totalSizeLimMb);	

	// time period to wait until closing an unused database file
	// this is to allow manual move or deletion of files
	// if != 0, then this value is limited in its lower bound by the transactionTimeout
	// default is 5000ms, a value <= 0 disables auto closing unused files
	int autoCloseDatabaseTimeout() const;
	void setAutoCloseDatabaseTimeout(const int& timeoutMs);

	// time period between opening and commiting a database transaction
	// this is implemented for performance reasons
	// default is 1000ms, a value <= 0 disables the use of transactions
	int transactionTimeout() const;
	void setTransactionTimeout(const int &timeoutMs);
	
	// required API for QUaServer::setHistorizer
	// write data point to backend, return true on success
	bool writeHistoryData(
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// update an existing node's data point in backend, return true on success
	bool updateHistoryData(
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// remove an existing node's data points within a range, return true on success
	bool removeHistoryData(
		const QUaNodeId &nodeId,
		const QDateTime& timeStart,
		const QDateTime& timeEnd,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the timestamp of the first sample available for the given node
	QDateTime firstTimestamp(
		const QUaNodeId &nodeId,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the timestamp of the latest sample available for the given node
	QDateTime lastTimestamp(
		const QUaNodeId &nodeId,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return true if given timestamp is available for the given node
	bool hasTimestamp(
		const QUaNodeId &nodeId,
		const QDateTime& timestamp,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return a timestamp matching the criteria for the given node
	QDateTime findTimestamp(
		const QUaNodeId &nodeId,
		const QDateTime& timestamp,
		const QUaHistoryBackend::TimeMatch& match,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the number for data points within a time range for the given node
	quint64 numDataPointsInRange(
		const QUaNodeId &nodeId,
		const QDateTime& timeStart,
		const QDateTime& timeEnd,
		QQueue<QUaLog>& logOut
	);
	// required API for QUaServer::setHistorizer
	// return the numPointsToRead data points for the given node form the given start time
	QVector<QUaHistoryDataPoint> readHistoryData(
		const QUaNodeId &nodeId,
		const QDateTime& timeStart,
		const quint64& numPointsOffset,
		const quint64& numPointsToRead,
		QQueue<QUaLog>& logOut
	);

	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// write a event's data to backend
	bool writeHistoryEventsOfType(
		const QUaNodeId            &eventTypeNodeId,
		const QList<QUaNodeId>     &emittersNodeIds,
		const QUaHistoryEventPoint &eventPoint,
		QQueue<QUaLog>             &logOut
	);
	// get event types (node ids) for which there are events stored for the
	// given emitter
	QVector<QUaNodeId> eventTypesOfEmitter(
		const QUaNodeId &emitterNodeId,
		QQueue<QUaLog>  &logOut
	);
	// find a timestamp matching the criteria for the emitter and event type
	QDateTime findTimestampEventOfType(
		const QUaNodeId                    &emitterNodeId,
		const QUaNodeId                    &eventTypeNodeId,
		const QDateTime                    &timestamp,
		const QUaHistoryBackend::TimeMatch &match,
		QQueue<QUaLog>                     &logOut
	);
	// get the number for events within a time range for the given emitter and event type
	quint64 numEventsOfTypeInRange(
		const QUaNodeId &emitterNodeId,
		const QUaNodeId &eventTypeNodeId,
		const QDateTime &timeStart,
		const QDateTime &timeEnd,
		QQueue<QUaLog>  &logOut
	);
	// return the numPointsToRead events for the given emitter and event type,
	// starting from the numPointsOffset offset after given start time (pagination)
	QVector<QUaHistoryEventPoint> readHistoryEventsOfType(
		const QUaNodeId &emitterNodeId,
		const QUaNodeId &eventTypeNodeId,
		const QDateTime &timeStart,
		const quint64   &numPointsOffset,
		const quint64   &numPointsToRead,
		const QList<QUaBrowsePath> &columnsToRead,
		QQueue<QUaLog>  &logOut
	);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

private:
	QString m_strDatabasePath;
	QString m_strBaseName;
	QString m_strSuffix;
	QTimer  m_timerTransaction;
	int     m_timeoutTransaction;
	double  m_fileSizeLimMb;
	double  m_totalSizeLimMb;
	QQueue<QUaLog> m_deferedLogOut;
	QFileSystemWatcher m_watcher;
	QMetaObject::Connection m_fileWatchConn;
	QElapsedTimer m_watchingTimer;
	QElapsedTimer m_checkingTimer;
	int     m_timeoutAutoCloseDatabases;
	QTimer  m_timerAutoCloseDatabases;
	bool    m_deferTotalSizeCheck;
	QHash<uint, QDateTime> m_findTimestampCache;
	// data prepared statements cache
	struct DataPreparedStatements {
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
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// event type name prepared statement cache
	struct EventTypeNamePreparedStatements {
		QSqlQuery insertEventTypeName;
		QSqlQuery selectEventTypeName;
	};
	static QString eventTypesTable;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	struct DatabaseInfo {
		QString strFileName;
		QElapsedTimer autoCloseTimer;
		QHash<QUaNodeId, DataPreparedStatements> dataPrepStmts;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		// event type prepared statements cache
		QHash<QUaNodeId, QSqlQuery> eventTypePrepStmts;
		QHash<QString, EventTypeNamePreparedStatements> eventTypeNamePrepStmt;
		// emitter prepared statements cache
		QHash<QUaNodeId, QSqlQuery> emitterPrepStmts;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	};
	QMap<QDateTime, DatabaseInfo> m_dbFiles;
	// return SQL type in string form, for given Qt type (only QUaServer supported types)
	static QHash<int, QString> m_hashTypes;
	static QMetaType::Type QVariantToQtType(const QVariant& value);
	static const QString QtTypeToSqlType(const QMetaType::Type& qtType);

	// set current sqlite database file to read from or write to
	bool createNewDatabase(
		QQueue<QUaLog>& logOut
	);
	// get current sqlite database file to read from or write to
	DatabaseInfo& getMostRecentDbInfo(
		bool &ok,
		QQueue<QUaLog>& logOut
	);
	// get database handle, creates it if not already
	bool getOpenedDatabase(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// closes current database if it is opened
	bool closeDatabase(
		DatabaseInfo& dbInfo
	);
	// checks current database size and creates a new one if necessary
	bool checkDatabase(
		QQueue<QUaLog>& logOut
	);
	// looks at current target path for matching database files and loads them to map
	bool reloadMatchingFiles(
		const bool &warnToLog,
		QQueue<QUaLog>& logOut
	);

	// check table exists
	bool tableExists(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QString& tableName,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// check node id table exists
	bool tableDataByNodeIdExists(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create node history table
	bool createDataNodeTable(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		const QMetaType::Type& storeType,
		QQueue<QUaLog>& logOut
	);
	// insert new data point into node history table
	bool insertDataPoint(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// check if transaction is needed
	bool handleTransactions(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// prepare a generic statement
	bool prepareStmt(
		const DatabaseInfo& dbInfo,
		QSqlQuery &query,
		const QString &strStmt,
		QQueue<QUaLog>& logOut
	);
	// prepare statement to insert history data points
	bool dataPrepareAllStmts(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		QQueue<QUaLog>& logOut
	);

	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// check event type node id table exists
	bool tableEventTypeByNodeIdExists(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create event type history table
	bool createEventTypeTable(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId &eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		QQueue<QUaLog>& logOut
	);
	// prepare statement to insert history event of type
	bool eventTypePrepareStmt(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		QQueue<QUaLog>& logOut
	);
	// check event type name table exists
	bool tableEventTypeNameExists(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create event type name table
	bool createEventTypeNameTable(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// prepare statement to insert event type name
	bool eventTypeNamePrepareStmt(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// check emitter node id table exists
	bool tableEmitterByNodeIdExists(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& emitterNodeId,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create emitter history table
	bool createEmitterTable(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& emitterNodeId,
		QQueue<QUaLog>& logOut
	);
	// prepare statement to insert history event in emitter table
	bool emitterPrepareStmt(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& emitterNodeId,
		QQueue<QUaLog>& logOut
	);
	// insert new event, return unique key
	bool insertEventPoint(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		qint64 & outEventKey,
		QQueue<QUaLog>& logOut
	);
	// insert new event type name if not exists, return unique key
	bool selectOrInsertEventTypeName(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		qint64& outEventTypeKey,
		QQueue<QUaLog>& logOut
	);
	// insert new event in emitter table
	bool insertEventReferenceInEmitterTable(
		DatabaseInfo& dbInfo,
		QSqlDatabase& db,
		const QUaNodeId& emitterNodeId,
		const QDateTime& timestamp,
		qint64& outEventTypeKey,
		qint64& outEventKey,
		QQueue<QUaLog>& logOut
	);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

};

#endif // UA_ENABLE_HISTORIZING

#endif // QUAMULTISQLITEHISTORIZER_H
