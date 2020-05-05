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

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	// OffNormal Alarm Example

	auto motionSensor = objsFolder->addChild<QUaBaseObject>("motionSensor");

	auto moving = motionSensor->addBaseDataVariable("moving");
	moving->setWriteAccess(true);
	moving->setDataType(QMetaType::Bool);
	moving->setValue(false);

	auto motionAlarm = motionSensor->addChild<QUaOffNormalAlarm>("alarm");
	motionAlarm->setConditionName("Motion Sensor Alarm");
	motionAlarm->setInputNode(moving);
	motionAlarm->setNormalValue(false);
	motionAlarm->setConfirmRequired(true);

	// Level Alarm Example

	auto levelSensor = objsFolder->addChild<QUaBaseObject>("levelSensor");

	auto level = levelSensor->addBaseDataVariable("level");
	level->setWriteAccess(true);
	level->setDataType(QMetaType::Double);
	level->setValue(0.0);
	
	auto levelAlarm = levelSensor->addChild<QUaExclusiveLevelAlarm>("alarm");
	levelAlarm->setConditionName("Level Sensor Alarm");
	levelAlarm->setInputNode(level);

	levelAlarm->setHighLimitRequired(true);
	levelAlarm->setLowLimitRequired(true);
	levelAlarm->setHighLimit(10.0);
	levelAlarm->setLowLimit(-10.0);

	// Branches 
	
	// uncomment to support branches
	//motionAlarm->setBranchQueueSize(10);
	//levelAlarm->setBranchQueueSize(10);

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
	// enable event history on specific emitter nodes
	motionSensor->setEventHistoryRead(true);
	levelSensor->setEventHistoryRead(true);
	// enable event history on server node (all events)
	server.setEventHistoryRead(true);

	// uncomment to support historizing branches
	//motionAlarm->setHistorizingBranches(true);
	//levelAlarm->setHistorizingBranches(true);

#endif // UA_ENABLE_HISTORIZING

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	server.start();

	return a.exec(); 
}