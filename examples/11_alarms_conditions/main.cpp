#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#include <QUaOffNormalAlarm>
#include <QUaExclusiveLevelAlarm>
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#ifdef UA_ENABLE_HISTORIZING
#ifndef SQLITE_HISTORIZER
#include "./../10_historizing/quainmemoryhistorizer.h"
#else
#include "./../10_historizing/quasqlitehistorizer.h"
#endif // !SQLITE_HISTORIZER
#endif // UA_ENABLE_HISTORIZING

#include <QElapsedTimer>

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QObject::connect(&server, &QUaServer::logMessage,
	[](const QUaLog& log) {
		qDebug()
			<< "[" << log.timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
			<< "][" << log.level
			<< "][" << log.category
			<< "] :" << log.message;
	});

	QUaFolderObject* objsFolder = server.objectsFolder();

#ifdef UA_ENABLE_HISTORIZING
#ifndef SQLITE_HISTORIZER
	// set historizer (must live at least as long as the server)
	QUaInMemoryHistorizer historizer;
#else
	QUaSqliteHistorizer historizer;
	QQueue<QUaLog> logOut;
	if (!historizer.setSqliteDbName("history.sqlite", logOut))
	{
		for (auto log : logOut)
		{
			qDebug() << "[" << log.level << "] :" << log.message;
		}
		return -1;
	}
	historizer.setTransactionTimeout(2 * 1000); // db transaction every 2 secs
#endif // !SQLITE_HISTORIZER
	// set the historizer
	// NOTE : historizer must live at least as long as server
	server.setHistorizer(historizer);
	// enable event history on server node (all events)
	server.setEventHistoryRead(true);
#endif // UA_ENABLE_HISTORIZING

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	auto source = objsFolder->addChild<QUaBaseObject>("source");

	auto offnormal_alarm = source->addChild<QUaOffNormalAlarm>("offnormal_alarm");
	offnormal_alarm->setConfirmRequired(true);
	offnormal_alarm->Enable();
	offnormal_alarm->addMethod("setActive", [offnormal_alarm](bool active) {
		offnormal_alarm->setActive(active);
	});

	auto level = source->addBaseDataVariable("level");
	level->setValue(0.0);
	level->setDataType(QMetaType::Double);
	source->addMethod("setLevel", [level](double levelValue) {
		level->setValue(levelValue);
		emit level->valueChanged(levelValue);
	});
	
	auto level_alarm = source->addChild<QUaExclusiveLevelAlarm>("level_alarm");
	level_alarm->setHighHighLimitRequired(true);
	level_alarm->setHighLimitRequired(true);
	level_alarm->setLowLimitRequired(true);
	level_alarm->setLowLowLimitRequired(true);
	level_alarm->setHighHighLimit(100.0);
	level_alarm->setHighLimit(10.0);
	level_alarm->setLowLimit(-10.0);
	level_alarm->setLowLowLimit(-100.0);
	level_alarm->setInputNode(level);
	level_alarm->Enable();
	
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	source->addMethod("Test", [level](qint64 iters) {
		QElapsedTimer timer, total;
		total.start();
		qint64 mean = 0;
		for (qint64 i = 0; i < iters; i++)
		{
			timer.restart();

			level->setValue(11);
			emit level->valueChanged(11);
			level->setValue(0);
			emit level->valueChanged(0);

			mean += timer.elapsed();
		}

		qDebug() << "[TOTAL]" << total.elapsed() << "[ms]";
		auto meandbl = mean / (double)iters;
		qDebug() << "[MEAN]" << meandbl << "[ms]";
		return meandbl;
	});

	server.start();

	return a.exec(); 
}