#include "quasqliteserializer.h"

#include <QSqlError>
#include <QSqlRecord>

QHash<int, QString> QUaSqliteSerializer::m_hashTypes = {
	{QMetaType::Bool       , "BOOLEAN"},
	{QMetaType::Char       , "CHARACTER"},
	{QMetaType::SChar      , "TINYINT"},
	{QMetaType::UChar      , "TINYINT"},
	{QMetaType::Short      , "SMALLINT"},
	{QMetaType::UShort     , "SMALLINT"},
	{QMetaType::Int        , "MEDIUMINT"},
	{QMetaType::UInt       , "MEDIUMINT"},
	{qMetaTypeId<QUaDataType>(), "MEDIUMINT"},
	{QMetaType::Long       , "BIGINT"},
	{QMetaType::LongLong   , "BIGINT"},
	{QMetaType::ULong      , "UNSIGNED BIG INT"},
	{QMetaType::ULongLong  , "UNSIGNED BIG INT"},
	{QMetaType::Float      , "REAL"},
	{QMetaType::Double     , "DOUBLE"},
	{QMetaType::QString    , "TEXT"},
	{QMetaType::QDateTime  , "DATETIME"},
	{QMetaType::QUuid      , "NUMERIC"},
	{QMetaType::QByteArray , "BLOB"},
	{QMetaType::UnknownType, "BLOB"}
};

QUaSqliteSerializer::QUaSqliteSerializer()
{

}

QString QUaSqliteSerializer::sqliteDbName() const
{
	return m_strSqliteDbName;
}

bool QUaSqliteSerializer::setSqliteDbName(
	const QString& strSqliteDbName,
	QQueue<QUaLog>& logOut)
{
	// set internally
	m_strSqliteDbName = strSqliteDbName;
	// create database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// success
	return true;
}

bool QUaSqliteSerializer::writeInstance(
	const QString& nodeId,
	const QString& typeName,
	const QMap<QString, QVariant>& attrs,
	const QList<QUaForwardReference>& forwardRefs,
	QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// check nodes table exists
	bool nodeTableExists;
	if (!this->tableExists(db, "QUaNode", nodeTableExists, logOut))
	{
		return false;
	}
	if (!nodeTableExists)
	{
		if (!this->createNodesTable(db, logOut))
		{
			return false;
		}
	}
	// check references table exists
	bool refsTableExists;
	if (!this->tableExists(db, "QUaForwardReference", refsTableExists, logOut))
	{
		return false;
	}
	if (!refsTableExists)
	{
		if (!this->createReferencesTable(db, logOut))
		{
			return false;
		}
	}
	// check type table exists
	bool typeTableExists;
	if (!this->tableExists(db, typeName, typeTableExists, logOut))
	{
		return false;
	}
	if (!typeTableExists)
	{
		if (!this->createTypeTable(db, typeName, attrs, logOut))
		{
			return false;
		}
	}
	// update or insert
	bool nodeExists;
	qint32 instanceKey;
	if (!this->nodeIdInTypeTable(db, typeName, nodeId, nodeExists, instanceKey, logOut))
	{
		return false;
	}
	if (nodeExists)
	{
		// update existing instance in type table

		// get existing references

		// merge references (to remove, to update, to add)

		// TODO : implement

	}
	else
	{
		// insert new node
		qint32 nodeKey = -1;
		if (!this->insertNewNode(db, nodeKey, logOut))
		{
			return false;
		}
		// insert new instance
		if (!this->insertNewInstance(db, typeName, nodeId, nodeKey, attrs, instanceKey, logOut))
		{
			return false;
		}
		// add references
		if (!this->addReferences(db, nodeKey, forwardRefs, logOut))
		{
			return false;
		}
	}
	// success
	return true;
}

bool QUaSqliteSerializer::readInstance(
	const QString& nodeId,
	QString& typeName,
	QMap<QString, QVariant>& attrs,
	QList<QUaForwardReference>& forwardRefs,
	QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}



	// success
	return true;
}

bool QUaSqliteSerializer::getOpenedDatabase(
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
			QObject::tr("Error opening %1. %2")
				.arg(m_strSqliteDbName)
				.arg(db.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	return true;
}

bool QUaSqliteSerializer::tableExists(
	QSqlDatabase& db, 
	const QString& strTableName, 
	bool& tableExists, 
	QQueue<QUaLog>& logOut)
{
	Q_UNUSED(logOut);
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT name FROM sqlite_master "
		"WHERE type='table' AND name='%1' COLLATE NOCASE"
	).arg(strTableName);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
		QObject::tr("Error checking %1 table exists in %2 database. Sql : %3.")
				.arg(strTableName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	if (query.next())
	{
		tableExists = true;
	}
	else
	{
		tableExists = false;
	}
	return true;
}

bool QUaSqliteSerializer::createNodesTable(
	QSqlDatabase& db, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt(
		"CREATE TABLE \"QUaNode\""
		"("
			"[QUaNodeId] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL"
		");"
	);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
		QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg("QUaNode")
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	return true;
}

bool QUaSqliteSerializer::createReferencesTable(
	QSqlDatabase& db, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt(
		"CREATE TABLE \"QUaForwardReference\""
		"("
			"[QUaForwardReference] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
			"[QUaNodeIdSource] INTEGER NOT NULL,"
			"[forwardName] NVARCHAR(100) NOT NULL,"
			"[inverseName] NVARCHAR(100) NOT NULL,"
			"[targetType] NVARCHAR(100) NOT NULL,"
			"[nodeIdTarget] NVARCHAR(100) NOT NULL,"
			"FOREIGN KEY ([QUaNodeIdSource]) REFERENCES \"QUaNode\" ([QUaNodeId]) ON DELETE NO ACTION ON UPDATE NO ACTION"
		");"
	);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
		QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg("QUaForwardReference")
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	return true;
}

bool QUaSqliteSerializer::createTypeTable(
	QSqlDatabase& db, 
	const QString& typeName, 
	const QMap<QString, QVariant>& attrs,
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	const QList<QString> attrNames = attrs.keys();
	QString strStmt = QString(
		"CREATE TABLE \"%1\""
		"("
			"[%1] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
			"[QUaNodeId] INTEGER NOT NULL,"
			"[nodeId] NVARCHAR(100) NOT NULL,"
	).arg(typeName);
	for (auto attrName : attrNames)
	{
		strStmt += QString("[%1] %2,") //  NOT NULL
			.arg(attrName)
			.arg(QUaSqliteSerializer::QtTypeToSqlType(
				static_cast<QMetaType::Type>(
					attrs[attrName].type() < 1024 ? 
					attrs[attrName].type() :
					attrs[attrName].userType()
				)
			));
	}
	strStmt += "FOREIGN KEY ([QUaNodeId]) REFERENCES \"QUaNode\" ([QUaNodeId]) "
		       "ON DELETE NO ACTION ON UPDATE NO ACTION"
	           ");";
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
		QObject::tr("Could not create %1 table in %2 database. Sql : %3.")
				.arg(typeName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	return true;
}

bool QUaSqliteSerializer::nodeIdInTypeTable(
	QSqlDatabase& db, 
	const QString& typeName, 
	const QString& nodeId, 
	bool& nodeExists, 
	qint32& instanceKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	nodeExists = false;
	QString strStmt = QString(
		"SELECT "
			"i.QUaNodeId "
		"FROM "
			"%1 i "
		"WHERE i.nodeId = '%2';"
	).arg(typeName).arg(nodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
		QObject::tr("Error querying %1 table in %2 database. Sql : %3.")
				.arg(typeName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	int keyFieldNo = query.record().indexOf(typeName);
	if (query.next())
	{
		nodeExists  = true;
		instanceKey = query.value(keyFieldNo).toInt();
	}
	else
	{
		nodeExists = false;
	}
	return true;
}

bool QUaSqliteSerializer::insertNewNode(
	QSqlDatabase& db, 
	qint32& nodeKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt(
		"INSERT INTO QUaNode DEFAULT VALUES;"
	);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
		QObject::tr("Could not insert new row in QUaNode table in %1 database. Sql : %2.")
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// get new key
	nodeKey = query.lastInsertId().toInt();
	return true;
}

bool QUaSqliteSerializer::insertNewInstance(
	QSqlDatabase& db, 
	const QString& typeName, 
	const QString& nodeId, 
	const qint32& nodeKey, 
	const QMap<QString, QVariant>& attrs, 
	qint32& instanceKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"INSERT INTO %1 ("
	).arg(typeName);
	QList<QString> attrNames = attrs.keys();
	attrNames.prepend("nodeId");
	attrNames.prepend("QUaNodeId");
	for (auto it = attrNames.begin(); it != attrNames.end(); ++it)
	{
		strStmt += std::next(it) != attrNames.end() ?
			QString("%1, ").arg(*it) : QString("%1) ").arg(*it);
	}
	strStmt += "VALUES (";
	for (auto it = attrNames.begin(); it != attrNames.end(); ++it)
	{
		strStmt += std::next(it) != attrNames.end() ?
			QString(":%1, ").arg(*it) : QString(":%1);").arg(*it);
	}
	query.prepare(strStmt);
	query.bindValue(0, nodeKey);
	query.bindValue(1, nodeId);
	for (int i = 2; i < attrNames.count(); i++)
	{
		query.bindValue(i, attrs.value(attrNames.at(i)));
	}
	if (!query.exec())
	{
		logOut << QUaLog({
		QObject::tr("Could not insert new row in %1 table in %2 database. Sql : %3.")
				.arg(typeName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	return true;
}

bool QUaSqliteSerializer::addReferences(
	QSqlDatabase& db, 
	const qint32& nodeKey, 
	const QList<QUaForwardReference>& forwardRefs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt(
		"INSERT INTO QUaForwardReference "
		"(QUaNodeIdSource, forwardName, inverseName, targetType, nodeIdTarget) "
		"VALUES "
		"(:QUaNodeIdSource, :forwardName, :inverseName, :targetType, :nodeIdTarget);"
	);
	query.prepare(strStmt);
	for (auto forwRef : forwardRefs)
	{
		query.bindValue(0, nodeKey);
		query.bindValue(1, forwRef.refType.strForwardName);
		query.bindValue(2, forwRef.refType.strInverseName);
		query.bindValue(3, forwRef.targetType);
		query.bindValue(4, forwRef.nodeIdTarget);
		if (!query.exec())
		{
			logOut << QUaLog({
				QObject::tr("Could not insert new row in QUaForwardReference table in %1 database. Sql : %2.")
						.arg(m_strSqliteDbName)
						.arg(query.lastError().text()),
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
			});
			return false;
		}
	}
	return true;
}

const QString QUaSqliteSerializer::QtTypeToSqlType(const QMetaType::Type& qtType)
{
	Q_ASSERT_X(QUaSqliteSerializer::m_hashTypes.contains(qtType), "QtTypeToSqlType", "Unknown type.");
	return QUaSqliteSerializer::m_hashTypes.value(qtType, "BLOB");
}
