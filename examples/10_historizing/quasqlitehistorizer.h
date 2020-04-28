#ifndef QUASQLITEHISTORIZER_H
#define QUASQLITEHISTORIZER_H

#ifdef UA_ENABLE_HISTORIZING

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
		QQueue<QUaLog>  &logOut
	);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

private:
	QString m_strSqliteDbName;
	QTimer  m_timerTransaction;
	int     m_timeoutTransaction;
	QQueue<QUaLog> m_deferedLogOut;
	// get database handle, creates it if not already
	bool getOpenedDatabase(
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	) const;
	// check table exists
	bool tableExists(
		QSqlDatabase& db,
		const QString& tableName,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// check node id table exists
	bool tableDataByNodeIdExists(
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create node history table
	bool createDataNodeTable(
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		const QMetaType::Type& storeType,
		QQueue<QUaLog>& logOut
	);
	// insert new data point into node history table
	bool insertDataPoint(
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		const QUaHistoryDataPoint& dataPoint,
		QQueue<QUaLog>& logOut
	);
	// check if transaction is needed
	bool handleTransactions(
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// prepare a generic statement
	bool prepareStmt(
		QSqlQuery &query,
		const QString &strStmt,
		QQueue<QUaLog>& logOut
	);
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
	QHash<QUaNodeId, DataPreparedStatements> m_dataPrepStmts;
	// prepare statement to insert history data points
	bool dataPrepareAllStmts(
		QSqlDatabase& db,
		const QUaNodeId &nodeId,
		QQueue<QUaLog>& logOut
	);

	// return SQL type in string form, for given Qt type (only QUaServer supported types)
	static QHash<int, QString> m_hashTypes;
	static QMetaType::Type QVariantToQtType(const QVariant& value);
	static const QString QtTypeToSqlType(const QMetaType::Type& qtType);

	// event history support
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// check event type node id table exists
	bool tableEventTypeByNodeIdExists(
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create event type history table
	bool createEventTypeTable(
		QSqlDatabase& db,
		const QUaNodeId &eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		QQueue<QUaLog>& logOut
	);
	// prepare statement to insert history event of type
	bool eventTypePrepareStmt(
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		QQueue<QUaLog>& logOut
	);
	// event type prepared statements cache
	QHash<QUaNodeId, QSqlQuery> m_eventTypePrepStmts;
	// check event type name table exists
	bool tableEventTypeNameExists(
		QSqlDatabase& db,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create event type name table
	bool createEventTypeNameTable(
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// prepare statement to insert event type name
	bool eventTypeNamePrepareStmt(
		QSqlDatabase& db,
		QQueue<QUaLog>& logOut
	);
	// event type name prepared statement cache
	struct EventTypeNamePreparedStatements {
		QSqlQuery insertEventTypeName;
		QSqlQuery selectEventTypeName;
	};
	QHash<QString, EventTypeNamePreparedStatements> m_eventTypeNamePrepStmt;
	static QString eventTypesTable;
	// check emitter node id table exists
	bool tableEmitterByNodeIdExists(
		QSqlDatabase& db,
		const QUaNodeId& emitterNodeId,
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create emitter history table
	bool createEmitterTable(
		QSqlDatabase& db,
		const QUaNodeId& emitterNodeId,
		QQueue<QUaLog>& logOut
	);
	// prepare statement to insert history event in emitter table
	bool emitterPrepareStmt(
		QSqlDatabase& db,
		const QUaNodeId& emitterNodeId,
		QQueue<QUaLog>& logOut
	);
	// emitter prepared statements cache
	QHash<QUaNodeId, QSqlQuery> m_emitterPrepStmts;
	// insert new event, return unique key
	bool insertEventPoint(
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		const QUaHistoryEventPoint& eventPoint,
		qint64 & outEventKey,
		QQueue<QUaLog>& logOut
	);
	// insert new event type name if not exists, return unique key
	bool selectOrInsertEventTypeName(
		QSqlDatabase& db,
		const QUaNodeId& eventTypeNodeId,
		qint64& outEventTypeKey,
		QQueue<QUaLog>& logOut
	);
	// insert new event in emitter table
	bool insertEventReferenceInEmitterTable(
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

#endif // QUASQLITEHISTORIZER_H
