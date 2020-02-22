#include "quasqliteserializer.h"

#include <QSqlError>

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
	if (this->tableExists(db, "QUaNode", nodeTableExists, logOut))
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
		qint32 nodeKey;
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
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(
		QString(
		"SELECT name FROM sqlite_master "
		"WHERE type='table' AND name='%1' COLLATE NOCASE"
		).arg(strTableName)
	);
	if (query.next())
	{
		QString strExists = query.value(0).toString();
		if (strExists.isEmpty())
		{
			logOut << QUaLog({
			QObject::tr("Table %1 was not found in %2 database.")
					.arg(strTableName)
					.arg(m_strSqliteDbName),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			});
			return false;
		}
	}
	return true;
}

bool QUaSqliteSerializer::createNodesTable(
	QSqlDatabase& db, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query;
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
	QSqlQuery query;
	QString strStmt(
		"CREATE TABLE \"QUaForwardReference\""
		"("
			"[QUaForwardReference] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
			"[QUaNodeIdSource] INTEGER NOT NULL,"
			"[forwardName] NVARCHAR(100) NOT NULL,"
			"[reverseName] NVARCHAR(100) NOT NULL,"
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
	const QList<QString> attrNames = attrs.keys();
	QSqlQuery query;
	QString strStmt = QString(
		"CREATE TABLE \"%1\""
		"("
			"[%1] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
			"[QUaNodeId] INTEGER NOT NULL,"
			"[nodeId] NVARCHAR(100) NOT NULL,"
	).arg(typeName);
	for (auto attrName : attrNames)
	{
		strStmt += QString("[%1] %2 NOT NULL,")
			.arg(attrName)
			.arg(QUaSqliteSerializer::QtTypeToSqlType(
				static_cast<QMetaType::Type>(attrs[attrName].type())
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
	QSqlQuery query;
	QString strStmt(
		""
	);

	// TODO : select statement

}

const QString QUaSqliteSerializer::QtTypeToSqlType(const QMetaType::Type& qtType)
{
	switch (qtType)
	{
		case QMetaType::Bool:
		{
			return "BOOLEAN";
		}
		break;
		case QMetaType::Char:
		{
			return "CHARACTER";
		}
		break;
		case QMetaType::SChar:
		case QMetaType::UChar:
		{
			return "TINYINT";
		}
		break;
		case QMetaType::Short:
		case QMetaType::UShort:
		{
			return "SMALLINT";
		}
		break;
		case QMetaType::Int:
		case QMetaType::UInt:
		{
			return "MEDIUMINT";
		}
		break;
		case QMetaType::Long:
		case QMetaType::LongLong:
		{
			return "BIGINT";
		}
		break;
		case QMetaType::ULong:
		case QMetaType::ULongLong:
		{
			return "UNSIGNED BIG INT";
		}
		break;
		case QMetaType::Float:
		{
			return "REAL";
		}
		break;
		case QMetaType::Double:
		{
			return "DOUBLE";
		}
		break;
		case QMetaType::QString:
		{
			return "TEXT";
		}
		break;
		case QMetaType::QDateTime:
		{
			return "DATETIME";
		}
		break;
		case QMetaType::QUuid:
		{
			return "NUMERIC";
		}
		break;
		case QMetaType::QByteArray:
		case QMetaType::UnknownType:
		{
			return "BLOB";
		}
		break;
		default:
		{
			Q_ASSERT(false);
		}
		break;
	}
	return "BLOB";
}
