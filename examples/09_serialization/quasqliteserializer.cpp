#include "quasqliteserializer.h"

#include <QSqlError>
#include <QSqlRecord>

// map supported types
QHash<int, QString> QUaSqliteSerializer::m_hashTypes = {
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
	bool nodeExists = false;
	qint32 nodeKey = -1;
	if (!this->nodeIdInTypeTable(db, typeName, nodeId, nodeExists, nodeKey, logOut))
	{
		return false;
	}
	if (nodeExists)
	{
		// update existing instance in type table
		if (!this->updateInstance(db, typeName, nodeKey, attrs, logOut))
		{
			return false;
		}
		// get existing references
		QList<QUaForwardReference> existingRefs;
		if (!this->nodeReferences(db, nodeKey, existingRefs, logOut))
		{
			return false;
		}
		// merge references (compute references to remove and to add)
		QList<QUaForwardReference> copyRefs(forwardRefs);
		// remove common references (do not need to be updated)
		existingRefs.erase(
		std::remove_if(existingRefs.begin(), existingRefs.end(),
		[&copyRefs](const QUaForwardReference &ref) {
			bool isCommon = copyRefs.contains(ref);
			copyRefs.removeAll(ref);
			return isCommon;
		}),
		existingRefs.end());
		// now existingRefs contains references to be removed
		if (!this->removeReferences(db, nodeKey, existingRefs, logOut))
		{
			return false;
		}
		// and copyRefs contains references to be added
		if (!this->addReferences(db, nodeKey, copyRefs, logOut))
		{
			return false;
		}
	}
	else
	{
		// insert new node
		if (!this->insertNewNode(db, nodeId, typeName, nodeKey, logOut))
		{
			return false;
		}
		// insert new instance
		if (!this->insertNewInstance(db, typeName, nodeKey, attrs, logOut))
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
	// get type name (table) and node key by node id
	qint32 nodeKey;
	if (!this->typeAndKeyInNodesTable(db, nodeId, typeName, nodeKey, logOut))
	{
		return false;
	}
	// get attributes
	if (!this->nodeAttributes(db, typeName, nodeKey, attrs, logOut))
	{
		return false;
	}
	// get references
	if (!this->nodeReferences(db, nodeKey, forwardRefs, logOut))
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
			QObject::tr("Error opening %1. Sql : %2")
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
			"[QUaNodeId] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
			"[typeName] NVARCHAR(100) NOT NULL, "
			"[nodeId] NVARCHAR(100) NOT NULL"
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
			"[QUaNodeId] INTEGER NOT NULL,"
			"[forwardName] NVARCHAR(100) NOT NULL,"
			"[inverseName] NVARCHAR(100) NOT NULL,"
			"[targetType] NVARCHAR(100) NOT NULL,"
			"[targetNodeId] NVARCHAR(100) NOT NULL,"
			"FOREIGN KEY ([QUaNodeId]) REFERENCES \"QUaNode\" ([QUaNodeId]) ON DELETE NO ACTION ON UPDATE NO ACTION"
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
	).arg(typeName);
	for (auto attrName : attrNames)
	{
		strStmt += QString("[%1] %2, ") // NOT NULL
			.arg(attrName)
			.arg(QUaSqliteSerializer::QtTypeToSqlType(
				static_cast<QMetaType::Type>(
					attrs[attrName].type() < 1024 ? 
					attrs[attrName].type() :
					attrs[attrName].userType())
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
	qint32& nodeKey,
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	nodeExists = false;
	QString strStmt = QString(
		"SELECT "
			"t.QUaNodeId AS QUaNodeId "
		"FROM "
			"%1 t "
		"INNER JOIN "
			"QUaNode n "
		"ON "
			"t.QUaNodeId = n.QUaNodeId "
		"WHERE "
			"n.nodeId = '%2';"
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
	if (query.next())
	{
		nodeExists  = true;
		// get node key
		QSqlRecord rec = query.record();
		int nodeKeyCol = rec.indexOf("QUaNodeId");
		Q_ASSERT(nodeKeyCol >= 0);
		nodeKey = query.value(nodeKeyCol).toInt();
	}
	else
	{
		nodeExists = false;
	}
	return true;
}

bool QUaSqliteSerializer::insertNewNode(
	QSqlDatabase& db, 
	const QString& nodeId,
	const QString& typeName,
	qint32& nodeKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt(
		"INSERT INTO QUaNode (typeName, nodeId) VALUES (:typeName, :nodeId);"
	);
	query.prepare(strStmt);
	query.bindValue(0, typeName);
	query.bindValue(1, nodeId);
	if (!query.exec())
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
	const qint32& nodeKey, 
	const QMap<QString, QVariant>& attrs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"INSERT INTO %1 ("
	).arg(typeName);
	QList<QString> attrNames = attrs.keys();
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
	for (int i = 1; i < attrNames.count(); i++)
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
		"(QUaNodeId, forwardName, inverseName, targetType, targetNodeId) "
		"VALUES "
		"(:QUaNodeId, :forwardName, :inverseName, :targetType, :targetNodeId);"
	);
	query.prepare(strStmt);
	for (auto forwRef : forwardRefs)
	{
		query.bindValue(0, nodeKey);
		query.bindValue(1, forwRef.refType.strForwardName);
		query.bindValue(2, forwRef.refType.strInverseName);
		query.bindValue(3, forwRef.targetType);
		query.bindValue(4, forwRef.targetNodeId);
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

bool QUaSqliteSerializer::removeReferences(
	QSqlDatabase& db, 
	const qint32& nodeKey, 
	const QList<QUaForwardReference>& forwardRefs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt(
		"DELETE FROM QUaForwardReference WHERE "
		"UaNodeId = :UaNodeId "
		"AND forwardName = :forwardName AND inverseName = :inverseName "
		"AND targetType = :targetType AND targetNodeId = :targetNodeId;"
	);
	query.prepare(strStmt);
	for (auto forwRef : forwardRefs)
	{
		query.bindValue(0, nodeKey);
		query.bindValue(1, forwRef.refType.strForwardName);
		query.bindValue(2, forwRef.refType.strInverseName);
		query.bindValue(3, forwRef.targetType);
		query.bindValue(4, forwRef.targetNodeId);
		if (!query.exec())
		{
			logOut << QUaLog({
				QObject::tr("Could not remove row in QUaForwardReference table in %1 database. Sql : %2.")
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

bool QUaSqliteSerializer::updateInstance(
	QSqlDatabase& db, 
	const QString& typeName, 
	const qint32& nodeKey, 
	const QMap<QString, QVariant>& attrs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"UPDATE %1 SET "
	).arg(typeName);
	QList<QString> attrNames = attrs.keys();
	for (auto it = attrNames.begin(); it != attrNames.end(); ++it)
	{
		strStmt += std::next(it) != attrNames.end() ?
			QString("%1 = :%1, ").arg(*it) : QString("%1 = :%1 ").arg(*it);
	}
	strStmt += QString("WHERE QUaNodeId = %1;").arg(nodeKey);
	query.prepare(strStmt);
	for (int i = 0; i < attrNames.count(); i++)
	{
		query.bindValue(i, attrs.value(attrNames.at(i)));
	}
	if (!query.exec())
	{
		logOut << QUaLog({
			QObject::tr("Could not update row with QUaNodeId = %1 in %2 table in %3 database. Sql : %4.")
				.arg(nodeKey)
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

bool QUaSqliteSerializer::typeAndKeyInNodesTable(
	QSqlDatabase& db, 
	const QString& nodeId, 
	QString& typeName, 
	qint32& nodeKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT n.QUaNodeId, n.typeName FROM QUaNode n WHERE n.nodeId = '%1';"
	).arg(nodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Failed to query %1 node id on QUaNode table in %2 database. Sql : %3.")
				.arg(nodeId)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	QSqlRecord rec  = query.record();
	int nodeKeyCol  = rec.indexOf("QUaNodeId");
	int typeNameCol = rec.indexOf("typeName");
	Q_ASSERT(nodeKeyCol >= 0 && typeNameCol >= 0);
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Node id %1 does not exist on QUaNode table in %2 database.")
				.arg(nodeId)
				.arg(m_strSqliteDbName),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
	}
	nodeKey  = query.value(nodeKeyCol).toInt();
	typeName = query.value(typeNameCol).toString();
	return true;
}

bool QUaSqliteSerializer::nodeAttributes(
	QSqlDatabase& db, 
	const QString& typeName, 
	const qint32& nodeKey, 
	QMap<QString, QVariant>& attrs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT * FROM %1 t WHERE t.QUaNodeId = %2;"
	).arg(typeName).arg(nodeKey);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Failed to query %1 key on %2 table in %3 database. Sql : %4.")
				.arg(nodeKey)
				.arg(typeName)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Node key %1 does not exist on %2 table in %3 database.")
				.arg(nodeKey)
				.arg(typeName)
				.arg(m_strSqliteDbName),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
	}
	// loop columns
	QSqlRecord rec = query.record();
	for (int i = 0; i < rec.count(); i++)
	{
		if (rec.fieldName(i).compare(typeName   , Qt::CaseSensitive) == 0 ||
			rec.fieldName(i).compare("QUaNodeId", Qt::CaseSensitive) == 0)
		{
			continue;
		}
		attrs[rec.fieldName(i)] = rec.value(i);
	}
	return true;
}

bool QUaSqliteSerializer::nodeReferences(
	QSqlDatabase& db, 
	const qint32& nodeKey, 
	QList<QUaForwardReference>& forwardRefs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT * FROM QUaForwardReference r WHERE r.QUaNodeId = %2;"
	).arg(nodeKey);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Failed to query %1 key on QUaForwardReference table in %2 database. Sql : %3.")
				.arg(nodeKey)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	while (query.next())
	{
		QSqlRecord rec  = query.record();
		int forwNameCol = rec.indexOf("forwardName");
		int invNameCol  = rec.indexOf("inverseName");
		int targTypeCol = rec.indexOf("targetType");
		int targNodeCol = rec.indexOf("targetNodeId");
		Q_ASSERT(forwNameCol >= 0 && invNameCol >= 0 && targTypeCol >= 0 && targNodeCol >= 0);
		forwardRefs << QUaForwardReference({
			query.value(targNodeCol).toString(),
			query.value(targTypeCol).toString(),
			{
				query.value(forwNameCol).toString(),
				query.value(invNameCol ).toString()
			}
		});
	}
	return true;
}

const QString QUaSqliteSerializer::QtTypeToSqlType(const QMetaType::Type& qtType)
{
	Q_ASSERT_X(
		QUaSqliteSerializer::m_hashTypes.contains(qtType), 
		"QtTypeToSqlType", "Unknown type."
	);
	return QUaSqliteSerializer::m_hashTypes.value(qtType, "BLOB");
}
