#ifndef QUASQLITESERIALIZER_H
#define QUASQLITESERIALIZER_H

#include <QUaServer>
#include <QSqlDatabase>
#include <QSqlQuery>

class QUaSqliteSerializer
{
public:
    QUaSqliteSerializer();

	// set sqlite database to read from or write to
	QString sqliteDbName() const;
	bool setSqliteDbName(
		const QString& strSqliteDbName,
		QQueue<QUaLog>& logOut
	);

	// required API for QUaNode::serialize
	bool writeInstance(
		const QString& nodeId,
		const QString& typeName,
		const QMap<QString, QVariant>& attrs,
		const QList<QUaForwardReference>& forwardRefs,
		QQueue<QUaLog>& logOut
	);

	// required API for QUaNode::deserialize
	bool readInstance(
		const QString& nodeId,
		QString& typeName,
		QMap<QString, QVariant>& attrs,
		QList<QUaForwardReference>& forwardRefs,
		QQueue<QUaLog>& logOut
	);

private:
	QString m_strSqliteDbName;
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
	// create nodes table
	bool createNodesTable(
		QSqlDatabase& db, 
		QQueue<QUaLog>& logOut
	);
	// create references table
	bool createReferencesTable(
		QSqlDatabase& db, 
		QQueue<QUaLog>& logOut
	);
	// create type table
	bool createTypeTable(
		QSqlDatabase& db,
		const QString& typeName, 
		const QMap<QString, QVariant>& attrs,
		QQueue<QUaLog>& logOut
	);
	// check if node already exists in type table
	bool nodeIdInTypeTable(
		QSqlDatabase& db,
		const QString& typeName, 
		const QString& nodeId, 
		bool& nodeExists,
		qint32& instanceKey,
		QQueue<QUaLog>& logOut);
	// insert new node, return unique key
	bool insertNewNode(
		QSqlDatabase& db, 
		qint32& nodeKey, 
		QQueue<QUaLog>& logOut
	);
	// insert new instance, return unique key
	bool insertNewInstance(
		QSqlDatabase& db,
		const QString& typeName, 
		const QString& nodeId, 
		const qint32& nodeKey,
		const QMap<QString, QVariant>& attrs,
		qint32& instanceKey,
		QQueue<QUaLog>& logOut
	);
	// add references
	bool addReferences(
		QSqlDatabase& db,
		const qint32& nodeKey,
		const QList<QUaForwardReference>& forwardRefs,
		QQueue<QUaLog>& logOut
	);

	// return SQL type in string form, for given Qt type (only QUaServer supported types)
	static const QString QtTypeToSqlType(const QMetaType::Type& qtType);
};

#endif // QUASQLITESERIALIZER_H
