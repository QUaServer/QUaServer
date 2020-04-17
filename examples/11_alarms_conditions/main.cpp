#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#include <QUaConditionVariable>
#include <QUaStateVariable>
#include <QUaTwoStateVariable>
#include <QUaCondition>
#include <QUaAcknowledgeableCondition>
#include <QUaAlarmCondition>
#include <QUaExclusiveLevelAlarm>
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	server.registerEnum<QUa::ExclusiveLimitState>();

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

	auto level = objsFolder->addBaseDataVariable("level");
	level->setValue(0.0);
	level->setDataType(QMetaType::Double);
	objsFolder->addMethod("setLevel", [level](double levelValue) {
		level->setValue(levelValue);
		emit level->valueChanged(levelValue);
	});
	
	auto level_alarm = objsFolder->addChild<QUaExclusiveLevelAlarm>("level_alarm");
	level_alarm->setHighHighLimitAllowed(true);
	level_alarm->setHighLimitAllowed(true);
	level_alarm->setLowLimitAllowed(true);
	level_alarm->setLowLowLimitAllowed(true);
	level_alarm->setHighHighLimit(100.0);
	level_alarm->setHighLimit(10.0);
	level_alarm->setLowLimit(-10.0);
	level_alarm->setLowLowLimit(-100.0);
	level_alarm->setInputNode(level);
	level_alarm->addMethod("setRetain", [level_alarm](bool retain) {
		level_alarm->setRetain(retain);
	});
	
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	server.start();

	return a.exec(); 
}
