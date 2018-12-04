// NOTE : needs to come first to avoid winsock redefinition errors
#include "open62541.h"

#include <QCoreApplication>
#include <QDebug>

#include <QConsoleListener>
#include <QLambdaThreadWorker>

/* predefined identifier for later use */
UA_NodeId pumpTypeId = { 1, UA_NODEIDTYPE_NUMERIC,{ 1001 } };

// Define Object Constructor
static UA_StatusCode pumpTypeConstructor(UA_Server      *server, 
	                                    const UA_NodeId *sessionId, 
	                                    void            *sessionContext,
	                                    const UA_NodeId *typeId,
	                                    void            *typeContext,
	                                    const UA_NodeId *nodeId, 
	                                    void            **nodeContext) {
	qDebug() << "Pump constructor called";
	/* Find the NodeId of the status child variable */

	// Define the relation between the pump Object and the status Variable
	UA_RelativePathElement rpe;
	UA_RelativePathElement_init(&rpe);
	rpe.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	rpe.isInverse       = false;
	rpe.includeSubtypes = false;
	rpe.targetName      = UA_QUALIFIEDNAME(1, (char*)"Status");
	// Define the path between the pump Object and the status Variable, based in relation
	UA_BrowsePath bp;
	UA_BrowsePath_init(&bp);
	bp.startingNode              = *nodeId;
	bp.relativePath.elementsSize = 1;
	bp.relativePath.elements     = &rpe;
	// Translate the path to a NodeId
	UA_BrowsePathResult bpr = UA_Server_translateBrowsePathToNodeIds(server, &bp);
	// Check translation result
	if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1)
	{
		return bpr.statusCode;
	}

	/* Set the status value */
	UA_Boolean status = true;
	UA_Variant value;
	UA_Variant_setScalar(&value, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
	UA_Server_writeValue(server, bpr.targets[0].targetId.nodeId, value);
	UA_BrowsePathResult_deleteMembers(&bpr);

	/* At this point we could replace the node context .. */

	return UA_STATUSCODE_GOOD;
}

// Create Object Types
static void defineObjectTypes(UA_Server *server) 
{
	/* Define the object type for "Device" */
	UA_NodeId deviceTypeId; /* get the nodeid assigned by the server */
	UA_ObjectTypeAttributes dtAttr = UA_ObjectTypeAttributes_default;
	dtAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"DeviceType");
	UA_Server_addObjectTypeNode(server, 
		                        UA_NODEID_NULL,
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		                        UA_QUALIFIEDNAME(1, (char*)"DeviceType"),
		                        dtAttr,
		                        NULL, 
		                        &deviceTypeId);

	UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
	mnAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"ManufacturerName");
	UA_NodeId manufacturerNameId;
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      deviceTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, (char*)"ManufacturerName"),
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
	modelAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"ModelName");
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      deviceTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, (char*)"ModelName"),
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), 
		                      modelAttr, 
		                      NULL, 
		                      NULL);

	/* Define the object type for "Pump" */
	UA_ObjectTypeAttributes ptAttr = UA_ObjectTypeAttributes_default;
	ptAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"PumpType");
	UA_Server_addObjectTypeNode(server, 
		                        pumpTypeId,
		                        deviceTypeId, 
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		                        UA_QUALIFIEDNAME(1, (char*)"PumpType"),
		                        ptAttr,
		                        NULL, 
		                        NULL);

	UA_VariableAttributes statusAttr = UA_VariableAttributes_default;
	statusAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Status");
	statusAttr.valueRank = UA_VALUERANK_SCALAR;
	UA_NodeId statusId;
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      pumpTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, (char*)"Status"),
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
	rpmAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"MotorRPM");
	rpmAttr.valueRank = UA_VALUERANK_SCALAR;
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      pumpTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, (char*)"MotorRPMs"),
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), 
		                      rpmAttr, 
		                      NULL, 
		                      NULL);
	// Add constructor
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = pumpTypeConstructor;
	lifecycle.destructor  = NULL;
	UA_Server_setNodeTypeLifecycle(server, pumpTypeId, lifecycle);
}



static void addPumpObjectInstance(UA_Server *server, char *name) 
{
	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
	oAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", name);
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
			addPumpObjectInstance(server, (char*)"pump1");
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
