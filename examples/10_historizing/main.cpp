#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>

#include <QUaServer>

#ifdef UA_ENABLE_HISTORIZING
#ifndef SQLITE_HISTORIZER
#include "quainmemoryhistorizer.h"
#else
#include "quasqlitehistorizer.h"
#endif // !SQLITE_HISTORIZER
#endif // UA_ENABLE_HISTORIZING

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
#include "./../08_events/myevent.h"
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;
	QObject::connect(&server, &QUaServer::logMessage,
    [](const QUaLog &log) {
        qDebug() 
			<< "["   << log.timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
			<< "]["  << log.level
			<< "]["  << log.category
			<< "] :" << log.message;
    });

	// it is also possible to historize events
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// create event with server as originator
	auto srvEvt = server.createEvent<MyEvent>();
	srvEvt->setDisplayName("MyServerEvent");
	srvEvt->setSourceName("Server");
	srvEvt->setMessage("An event occured in the server");
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

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
	// add test variables
	QTimer timerVars;
	QUaFolderObject* objsFolder = server.objectsFolder();
	for (int i = 0; i < 10; i++)
	{
		// create int variable
		auto varInt = objsFolder->addBaseDataVariable(QString("Int%1").arg(i), QString("ns=1;s=Int%1").arg(i));
		// NOTE : must enable historizing for each variable
		varInt->setHistorizing(true);
		varInt->setReadHistoryAccess(true);
		varInt->setValue(0);
		// set random value
		QObject::connect(&timerVars, &QTimer::timeout, varInt, [varInt]() {
			varInt->setValue(QRandomGenerator::global()->generate());
		});
	}
	// update variable every half a second
	timerVars.start(500);

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	QTimer timerEvts;
	QObject::connect(&timerVars, &QTimer::timeout, srvEvt, [srvEvt]() {
		static quint32 counter = 0;
		auto time = QDateTime::currentDateTime();
		srvEvt->setMessage(QObject::tr("An event occured in the server %1").arg(++counter));
		srvEvt->setTime(time.toUTC());
		srvEvt->setReceiveTime(time.toUTC());
		srvEvt->trigger();
	});	
	// trigger event every second
	timerVars.start(1000);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

#endif // UA_ENABLE_HISTORIZING

	// start server
	server.start();

	return a.exec();
}
