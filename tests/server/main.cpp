// NOTE : needs to come first to avoid winsock redefinition errors
#include "open62541.h"

#include <QCoreApplication>
#include <QDebug>

#include <QConsoleListener>
#include <QLambdaThreadWorker>

/* predefined identifier for later use */
UA_NodeId pumpTypeId = { 1, UA_NODEIDTYPE_NUMERIC,{ 1001 } };

static void defineObjectTypes(UA_Server *server) 
{
	/* Define the object type for "Device" */
	UA_NodeId deviceTypeId; /* get the nodeid assigned by the server */
	UA_ObjectTypeAttributes dtAttr = UA_ObjectTypeAttributes_default;
	dtAttr.displayName = UA_LOCALIZEDTEXT("en-US", "DeviceType");
	UA_Server_addObjectTypeNode(server, 
		                        UA_NODEID_NULL,
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		                        UA_QUALIFIEDNAME(1, "DeviceType"), 
		                        dtAttr,
		                        NULL, 
		                        &deviceTypeId);

	UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
	mnAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ManufacturerName");
	UA_NodeId manufacturerNameId;
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      deviceTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, "ManufacturerName"),
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), 
		                      mnAttr, 
		                      NULL, 
		                      &manufacturerNameId);
	/* Make the manufacturer name mandatory */
	UA_Server_addReference(server, 
		                   manufacturerNameId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);

	UA_VariableAttributes modelAttr = UA_VariableAttributes_default;
	modelAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ModelName");
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      deviceTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, "ModelName"),
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), 
		                      modelAttr, 
		                      NULL, 
		                      NULL);

	/* Define the object type for "Pump" */
	UA_ObjectTypeAttributes ptAttr = UA_ObjectTypeAttributes_default;
	ptAttr.displayName = UA_LOCALIZEDTEXT("en-US", "PumpType");
	UA_Server_addObjectTypeNode(server, 
		                        pumpTypeId,
		                        deviceTypeId, 
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		                        UA_QUALIFIEDNAME(1, "PumpType"), 
		                        ptAttr,
		                        NULL, 
		                        NULL);

	UA_VariableAttributes statusAttr = UA_VariableAttributes_default;
	statusAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Status");
	statusAttr.valueRank = UA_VALUERANK_SCALAR;
	UA_NodeId statusId;
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      pumpTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, "Status"),
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), 
		                      statusAttr, 
		                      NULL, 
		                      &statusId);
	/* Make the status variable mandatory */
	UA_Server_addReference(server, 
		                   statusId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);

	UA_VariableAttributes rpmAttr = UA_VariableAttributes_default;
	rpmAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MotorRPM");
	rpmAttr.valueRank = UA_VALUERANK_SCALAR;
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      pumpTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, "MotorRPMs"),
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), 
		                      rpmAttr, 
		                      NULL, 
		                      NULL);
}

static void addPumpObjectInstance(UA_Server *server, char *name) 
{
	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
	oAttr.displayName = UA_LOCALIZEDTEXT("en-US", name);
	UA_Server_addObjectNode(server, 
		                    UA_NODEID_NULL,
		                    UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
		                    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		                    UA_QUALIFIEDNAME(1, name),
		                    pumpTypeId, /* this refers to the object type identifier */
		                    oAttr, 
		                    NULL, 
		                    NULL);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	// instantiate server objects
	UA_Boolean       running = false;
	UA_ServerConfig *config  = nullptr;
	UA_Server       *server  = nullptr;

    // create worker thread
    QLambdaThreadWorker thdworker;

	// create method to start server from main thread
	auto funcStartServer = [&thdworker, &server, &config, &running]() mutable {
		// early exit
		if (running)
		{
			qDebug() << "Server already running";
			return;
		}
		// run server in thread
		thdworker.execInThread([&server, &config, &running]() mutable {
			qDebug() << "Starting server in thread " << QThread::currentThread();
			// reset server running state
			config  = UA_ServerConfig_new_default();
			server  = UA_Server_new(config);
			running = true;
			// configure server
			defineObjectTypes(server);
			addPumpObjectInstance(server, "pump1");
			// run server (blocking)
			UA_StatusCode retval = UA_Server_run(server, &running);
			QString       strRetCode = UA_StatusCode_name(retval);
			// delete server
			UA_Server_delete(server);
			UA_ServerConfig_delete(config);
			qDebug() << "Server has been shut down with code " << strRetCode;
		});
	};
	// start server initially
	funcStartServer();

	// listen to console input
	QConsoleListener console;
	QObject::connect(&console, &QConsoleListener::newLine, [&a, &running, &funcStartServer](const QString &strNewLine) {
		qDebug() << "Echo :" << strNewLine;
		// kill server
		if (strNewLine.compare("x", Qt::CaseInsensitive) == 0)
		{
			qDebug() << "Shutting down server";
			// shutdown server
			running = false;
		}
		// start server
		if (strNewLine.compare("s", Qt::CaseInsensitive) == 0)
		{
			// try to start server
			funcStartServer();
		}
		// quit app
		if (strNewLine.compare("q", Qt::CaseInsensitive) == 0)
		{
			qDebug() << "Exiting app";
			// shutdown server
			running = false;
			// exit qt app
			a.quit();
		}
	});
	qDebug() << "Listening to console input in thread " << QThread::currentThread();
	
    return a.exec();
}
