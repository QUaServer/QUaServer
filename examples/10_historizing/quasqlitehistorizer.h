#ifndef QUASQLITEHISTORIZER_H
#define QUASQLITEHISTORIZER_H

#include <QUaHistoryBackend>
#include <QSqlDatabase>
#include <QSqlQuery>

class QUaSqliteHistorizer
{
public:

	// set sqlite database to read from or write to
	QString sqliteDbName() const;
	bool setSqliteDbName(
		const QString& strSqliteDbName,
		QQueue<QUaLog>& logOut
	);
	

private:
	QString m_strSqliteDbName;
	QHash<QString, QSqlQuery> m_prepStmts;
	// get database handle, creates it if not already
	bool getOpenedDatabase(
		QSqlDatabase &db, 
		QQueue<QUaLog>& logOut
	);
	// check table exists
	bool tableExists(
		QSqlDatabase& db,
		const QString& strTableName, 
		bool& tableExists,
		QQueue<QUaLog>& logOut
	);
	// create node history table
	bool createNodeTable(
		QSqlDatabase& db,
		const QString& strTableName,
		const QMetaType::Type& storeType,
		QQueue<QUaLog>& logOut
	);

	// return SQL type in string form, for given Qt type (only QUaServer supported types)
	static QHash<int, QString> m_hashTypes;
	static const QString QtTypeToSqlType(const QMetaType::Type& qtType);
};


#endif // QUASQLITEHISTORIZER_H
