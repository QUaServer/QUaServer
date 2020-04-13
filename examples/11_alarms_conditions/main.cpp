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

#include <QUaFiniteStateMachine>
#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	server.registerEnum<QUaStatus>();

	QObject::connect(&server, &QUaServer::logMessage,
		[](const QUaLog& log) {
			qDebug()
				<< "[" << log.timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
				<< "][" << log.level
				<< "][" << log.category
				<< "] :" << log.message;
		});
	QObject::connect(&server, &QUaServer::clientConnected,
		[](const QUaSession* sessionData)
		{
			qDebug() << "Connected" << sessionData->address() << ":" << sessionData->port() << "|" << sessionData->applicationName();
		});

	QUaFolderObject* objsFolder = server.objectsFolder();

	// NOTE : invalid namespace when ns > 1 ?

	auto var1 = objsFolder->addBaseDataVariable("var1", { 1, "var1" });
	var1->setWriteAccess(true);
	QUaQualifiedName someName(1, "whatever");
	var1->setValue(someName);
	Q_ASSERT(var1->value<QUaQualifiedName>() == someName);

	auto var2 = objsFolder->addBaseDataVariable("var2", "ns=0;s=var2");
	var2->setWriteAccess(true);
	QUaNodeId someNodeId(1, "whatever");
	var2->setValue(someNodeId);
	Q_ASSERT(var2->value<QUaNodeId>() == someNodeId);

	auto var3 = objsFolder->addBaseDataVariable("var3", { 1, "var3" });
	var3->setWriteAccess(true);
	QUaLocalizedText someLocalizedText("fr", "whatever");
	var3->setValue(someLocalizedText);
	Q_ASSERT(var3->value<QUaLocalizedText>() == someLocalizedText);

	auto var4 = objsFolder->addBaseDataVariable("var4", { 0, "var4" });
	var4->setWriteAccess(true);
	var4->setDataType(QMetaType::QDateTime);
	QVector<QUaNodeId> vectNodes = QVector<QUaNodeId>()
		<< var1->nodeId()
		<< var2->nodeId()
		<< var3->nodeId();
	var4->setValue(vectNodes);
	for (auto nodeId : var4->value<QVector<QUaNodeId>>())
	{
		qDebug() << nodeId.toXmlString();
	}

////#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS
////
////	auto srcObj = objsFolder->addChild<QUaBaseObject>("SourceObject", "ns=0;s=source_object");
////	srcObj->setEventNotifierSubscribeToEvents();
////	srcObj->addMethod("delete",
////	[srcObj]() {
////		delete srcObj;
////	});
////
////	auto condObj = srcObj->addChild<QUaAcknowledgeableCondition>("MyAcknowledgeableCondition");
////	condObj->setConfirmAllowed(true);
////	//condObj->addMethod("resetStates",
////	//[condObj]() {
////	//	condObj->resetInternals();
////	//});
////	condObj->addMethod("setRetain",
////	[condObj](bool retain) {
////		condObj->setRetain(retain);
////	});
////	condObj->addMethod("setQuality",
////	[condObj](QUaStatus quality) {
////		condObj->setQuality(quality);
////	});
////	condObj->addMethod("setSeverity",
////	[condObj](quint16 severity) {
////		condObj->setSeverity(severity);
////	});
////	condObj->addMethod("createBranch",
////	[condObj]() {
////		condObj->createBranch();
////	});
////	condObj->addMethod("delete",
////	[condObj]() {
////		delete condObj;
////	});
////
////	//auto almObj = objsFolder->addChild<QUaAlarmCondition>();
////	//almObj->setBrowseName("MyAlarmCondition");
////	//almObj->setDisplayName("MyAlarmCondition");
////	//almObj->setConfirmAllowed(true);
////	//almObj->addMethod("setActive",
////	//[almObj](bool active) {
////	//	almObj->setActive(active);
////	//});
////	//almObj->addMethod("setRetain",
////	//[almObj](bool retain) {
////	//	almObj->setRetain(retain);
////	//});
////
////#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

	server.start();

	return a.exec(); 
}
