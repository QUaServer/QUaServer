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

bool QUaSqliteSerializer::serializeStart(QQueue<QUaLog>& logOut)
{
	// cleanup
	m_prepStmts.clear();
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
	// start transaction
	if (!db.transaction())
	{
		logOut << QUaLog({
			QObject::tr("Failed to begin transaction in %1 database. Sql : %2.")
				.arg(m_strSqliteDbName)
				.arg(db.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	return true;
}

bool QUaSqliteSerializer::serializeEnd(QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// end transaction
	if (!db.commit())
	{
		logOut << QUaLog({
			QObject::tr("Failed to commit transaction in %1 database. Sql : %2.")
				.arg(m_strSqliteDbName)
				.arg(db.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// close database
	db.close();
	// cleanup
	m_prepStmts.clear();
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
		if (!this->nodeReferences(db, nodeId, existingRefs, logOut))
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
		if (!this->insertNewNode(db, nodeId, nodeKey, logOut))
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

bool QUaSqliteSerializer::deserializeStart(QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}

	// TODO : check default tables

	return true;
}

bool QUaSqliteSerializer::deserializeEnd(QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db;
	if (!this->getOpenedDatabase(db, logOut))
	{
		return false;
	}
	// close database
	db.close();
	return true;
}

bool QUaSqliteSerializer::readInstance(
	const QString& nodeId,
	const QString& typeName,
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
	// get attributes
	if (!this->nodeAttributes(db, typeName, nodeId, attrs, logOut))
	{
		return false;
	}
	// get references
	if (!this->nodeReferences(db, nodeId, forwardRefs, logOut))
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
	// save time by using cache instead of SQL
	Q_UNUSED(db);
	tableExists = m_prepStmts.contains(strTableName);
	return true;
	/*
	// query database for table
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
	*/
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
	// create prepared statement for insert
	strStmt = QString(
		"INSERT INTO QUaNode (nodeId) VALUES (:nodeId);"
	);
	if (!query.prepare(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error preparing statement %1 for %2 database. Sql : %3.")
				.arg(strStmt)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// cache prepared statement
	m_prepStmts["QUaNode"] = query;
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
	// create prepared statement for insert
	strStmt = QString(
		"INSERT INTO QUaForwardReference "
		"(QUaNodeId, forwardName, inverseName, targetType, targetNodeId) "
		"VALUES "
		"(:QUaNodeId, :forwardName, :inverseName, :targetType, :targetNodeId);"
	);
	if (!query.prepare(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error preparing statement %1 for %2 database. Sql : %3.")
				.arg(strStmt)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// cache prepared statement
	m_prepStmts["QUaForwardReference"] = query;
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
	QList<QString> attrNames = attrs.keys();
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
	// create prepared statement for insert
	strStmt = QString(
		"INSERT INTO %1 ("
	).arg(typeName);
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
	if (!query.prepare(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Error preparing statement %1 for %2 database. Sql : %3.")
				.arg(strStmt)
				.arg(m_strSqliteDbName)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// cache prepared statement
	m_prepStmts[typeName] = query;
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
	qint32& nodeKey, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	Q_ASSERT(m_prepStmts.contains("QUaNode"));
	QSqlQuery& query = m_prepStmts["QUaNode"];
	query.bindValue(0, nodeId);
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
	Q_ASSERT(m_prepStmts.contains(typeName));
	QList<QString> attrNames = attrs.keys();
	attrNames.prepend("QUaNodeId");
	QSqlQuery& query = m_prepStmts[typeName];
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
	Q_ASSERT(m_prepStmts.contains("QUaForwardReference"));
	QSqlQuery& query = m_prepStmts["QUaForwardReference"];
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

bool QUaSqliteSerializer::nodeAttributes(
	QSqlDatabase& db, 
	const QString& typeName, 
	const QString& nodeId,
	QMap<QString, QVariant>& attrs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT "
			"* "
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
			QObject::tr("Failed to query %1 node id on %2 table in %3 database. Sql : %4 : %5.")
				.arg(nodeId)
				.arg(typeName)
				.arg(m_strSqliteDbName)
				.arg(strStmt)
				.arg(query.lastError().text()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	if (!query.next())
	{
		logOut << QUaLog({
			QObject::tr("Node id %1 does not exist on %2 table in %3 database.")
				.arg(nodeId)
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
			rec.fieldName(i).compare("QUaNodeId", Qt::CaseSensitive) == 0 ||
			rec.fieldName(i).compare("nodeId"   , Qt::CaseSensitive) == 0)
		{
			continue;
		}
		attrs[rec.fieldName(i)] = rec.value(i);
	}
	return true;
}

bool QUaSqliteSerializer::nodeReferences(
	QSqlDatabase& db, 
	const QString& nodeId,
	QList<QUaForwardReference>& forwardRefs, 
	QQueue<QUaLog>& logOut)
{
	Q_ASSERT(db.isValid() && db.isOpen());
	QSqlQuery query(db);
	QString strStmt = QString(
		"SELECT "
			"* "
		"FROM "
			"QUaForwardReference r "
		"INNER JOIN "
			"QUaNode n "
		"ON "
			"r.QUaNodeId = n.QUaNodeId "
		"WHERE "
			"n.nodeId = '%1';"
	).arg(nodeId);
	if (!query.exec(strStmt))
	{
		logOut << QUaLog({
			QObject::tr("Failed to query %1 node id on QUaForwardReference table in %2 database. Sql : %3.")
				.arg(nodeId)
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
