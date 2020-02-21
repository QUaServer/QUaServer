#include "quasqliteserializer.h"

QUaSQLiteSerializer::QUaSQLiteSerializer()
{

}

QString QUaSQLiteSerializer::sqliteDbName() const
{
	return m_strSqliteDbName;
}

bool QUaSQLiteSerializer::setSqliteDbName(
	const QString& strSqliteDbName,
	QQueue<QUaLog>& logOut)
{
	// set internally
	m_strSqliteDbName = strSqliteDbName;
	// create database handle
	QSqlDatabase db = this->getDatabase(logOut);
	// check if opened correctly
	if (!db.isOpen())
	{
		// error
		return false;
	}
	// success
	return true;
}

bool QUaSQLiteSerializer::writeInstance(
	const QString& nodeId,
	const QString& typeName,
	const QMap<QString, QVariant>& attrs,
	const QList<QUaForwardReference>& forwardRefs,
	QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db = this->getDatabase(logOut);
	// check if opened correctly
	if (!db.isOpen())
	{
		// error
		return false;
	}



	// success
	return true;
}

bool QUaSQLiteSerializer::readInstance(
	const QString& nodeId,
	QString& typeName,
	QMap<QString, QVariant>& attrs,
	QList<QUaForwardReference>& forwardRefs,
	QQueue<QUaLog>& logOut)
{
	// get database handle
	QSqlDatabase db = this->getDatabase(logOut);
	// check if opened correctly
	if (!db.isOpen())
	{
		// error
		return false;
	}



	// success
	return true;
}

QSqlDatabase QUaSQLiteSerializer::getDatabase(QQueue<QUaLog>& logOut)
{
	QSqlDatabase db;
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
	}
}
