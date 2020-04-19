#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
#include <QUaOffNormalAlarm>
#include <QUaExclusiveLevelAlarm>
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

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

	auto source = objsFolder->addChild<QUaBaseObject>("source");

	auto offnormal_alarm = source->addChild<QUaOffNormalAlarm>("offnormal_alarm");
	offnormal_alarm->setRetain(true);
	offnormal_alarm->setConfirmAllowed(true);
	offnormal_alarm->Enable();
	offnormal_alarm->addMethod("setActive", [offnormal_alarm](bool active) {
		offnormal_alarm->setActive(active);
	});

	//auto level = source->addBaseDataVariable("level");
	//level->setValue(0.0);
	//level->setDataType(QMetaType::Double);
	//source->addMethod("setLevel", [level](double levelValue) {
	//	level->setValue(levelValue);
	//	emit level->valueChanged(levelValue);
	//});
	//
	//auto level_alarm = source->addChild<QUaExclusiveLevelAlarm>("level_alarm");
	//level_alarm->Enable();
	//level_alarm->setHighHighLimitAllowed(true);
	//level_alarm->setHighLimitAllowed(true);
	//level_alarm->setLowLimitAllowed(true);
	//level_alarm->setLowLowLimitAllowed(true);
	//level_alarm->setHighHighLimit(100.0);
	//level_alarm->setHighLimit(10.0);
	//level_alarm->setLowLimit(-10.0);
	//level_alarm->setLowLowLimit(-100.0);
	//level_alarm->setInputNode(level);
	//level_alarm->addMethod("setRetain", [level_alarm](bool retain) {
	//	level_alarm->setRetain(retain);
	//});
	
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	server.start();

	return a.exec(); 
}