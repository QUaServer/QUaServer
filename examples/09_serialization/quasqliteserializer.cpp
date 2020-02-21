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
		if (!this->createTypeTable(db, typeName, attrs.keys(), logOut))
		{
			return false;
		}
	}
	// update or insert
	bool nodeExists;
	if (!this->nodeIdInTypeTable(db, typeName, nodeId, nodeExists, logOut))
	{
		return false;
	}
	if (nodeExists)
	{
		// update

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
		qint32 instanceKey;
		if (!this->insertNewInstance(db, typeName, nodeId, nodeKey, attrs, instanceKey, logOut))
		{
			return false;
		}

		// TODO : implement add references

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
