#ifndef QUASQLITESERIALIZER_H
#define QUASQLITESERIALIZER_H

#include <QUaServer>
#include <QSqlDatabase>
#include <QSqlQuery>

class QUaSQLiteSerializer
{
public:
    QUaSQLiteSerializer();

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
	// get database handle, adds it if not added
	QSqlDatabase getDatabase(QQueue<QUaLog>& logOut);
};

#endif // QUASQLITESERIALIZER_H
