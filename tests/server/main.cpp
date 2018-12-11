// NOTE : needs to come first to avoid winsock redefinition errors
#include "open62541.h"

#include <QCoreApplication>
#include <QDebug>

#include <QConsoleListener>
#include <QLambdaThreadWorker>

/* predefined identifier for later use */
UA_NodeId pumpTypeId = { 1, UA_NODEIDTYPE_NUMERIC,{ 1001 } };

// Helper tofind object component by UA_QualifiedName
static UA_StatusCode findNodeIdComponentByQualifiedName(UA_Server              *server, 
	                                                    const UA_NodeId        *nodeId, 
	                                                    const UA_QualifiedName *componentQualName, 
	                                                    UA_NodeId              *componentNodeId)
{
	// Define the relation between the pump Object and the status Variable
	UA_RelativePathElement rpe;
	UA_RelativePathElement_init(&rpe);
	rpe.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	rpe.isInverse       = false;
	rpe.includeSubtypes = false;
	rpe.targetName      = *componentQualName;
	// Define the path between the pump Object and the status Variable, based in relation
	UA_BrowsePath bp;
	UA_BrowsePath_init(&bp);
	bp.startingNode = *nodeId;
	bp.relativePath.elementsSize = 1;
	bp.relativePath.elements = &rpe;
	// Translate the path to a NodeId
	UA_BrowsePathResult bpr = UA_Server_translateBrowsePathToNodeIds(server, &bp);
	// Check translation result
	if (bpr.statusCode != UA_STATUSCODE_GOOD || bpr.targetsSize < 1)
	{
		return bpr.statusCode;
	}
	*componentNodeId = bpr.targets[0].targetId.nodeId;
	UA_BrowsePathResult_deleteMembers(&bpr);
	return UA_STATUSCODE_GOOD;
}

// Define ObjectType "PumpType" Constructor
static UA_StatusCode pumpTypeConstructor(UA_Server      *server, 
	                                    const UA_NodeId *sessionId, 
	                                    void            *sessionContext,
	                                    const UA_NodeId *typeId,
	                                    void            *typeContext,
	                                    const UA_NodeId *nodeId,
	                                    void            **nodeContext) 
{
	qDebug() << "Pump constructor called";
	/* Find the NodeId of the "Status" child variable */
	UA_NodeId statusNodeId;
	UA_StatusCode retVal = findNodeIdComponentByQualifiedName(server, 
		                                                      nodeId, 
		                                                      &UA_QUALIFIEDNAME(1, (char*)"Status"), 
		                                                      &statusNodeId);
	if (retVal != UA_STATUSCODE_GOOD)
	{
		return retVal;
	}

	/* Set the default "Status" value */
	UA_Boolean status = false;
	UA_Variant valueStatus;
	UA_Variant_setScalar(&valueStatus, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
	UA_Server_writeValue(server, statusNodeId, valueStatus);

	/* Find the "MotorRPMs" of the status child variable */
	UA_NodeId rpmNodeId;
	retVal = findNodeIdComponentByQualifiedName(server,
		                                        nodeId,
		                                        &UA_QUALIFIEDNAME(1, (char*)"MotorRPMs"),
		                                        &rpmNodeId);
	if (retVal != UA_STATUSCODE_GOOD)
	{
		return retVal;
	}

	/* Set the default "MotorRPMs" value */
	UA_Double rpm = 0.0;
	UA_Variant valueRpm;
	UA_Variant_setScalar(&valueRpm, &rpm, &UA_TYPES[UA_TYPES_DOUBLE]);
	UA_Server_writeValue(server, rpmNodeId, valueRpm);

	/* At this point we could replace the node context .. */
	static int counter = 0;
	int * p_intContext = new int;
	*p_intContext = ++counter;
	// NOTE : UA_Server_setNodeContext(server, *nodeId, p_intContext); would not work, gets overwritten by context below
	*nodeContext = p_intContext;

	return UA_STATUSCODE_GOOD;
}

// Define ObjectType "PumpType" Destructor
static void pumpTypeDestructor(UA_Server       *server,
	                           const UA_NodeId *sessionId,
	                           void            *sessionContext,
	                           const UA_NodeId *typeId,
	                           void            *typeContext,
	                           const UA_NodeId *nodeId,
	                           void            **nodeContext) 
{	
	// cleanup context
	int * p_intContext = (int *)&nodeContext;
	qDebug() << "Pump destructor called : " << *p_intContext;
	delete p_intContext;
}

static UA_StatusCode pumpStartPumpMethod(
	UA_Server        *server,
	const UA_NodeId  *sessionId, 
	void             *sessionHandle,
	const UA_NodeId  *methodId, 
	void             *methodContext,
	const UA_NodeId  *objectId, 
	void             *objectContext,
	size_t            inputSize, 
	const UA_Variant *input,
	size_t            outputSize, 
	UA_Variant       *output) 
{

	/*
	// Alternative
	void *parentContext;
	UA_Server_getNodeContext(server, *objectId, &parentContext);
	const int * p_intObjContext = (const int *)(parentContext);
	*/
	const int * p_intObjContext = (const int *)objectContext;
	qDebug() << "Start Pump method called with object context : " << *p_intObjContext;

	UA_NodeId statusNodeId;
	UA_StatusCode retVal = findNodeIdComponentByQualifiedName(server, 
		                                                      objectId, 
		                                                      &UA_QUALIFIEDNAME(1, (char*)"Status"), 
		                                                      &statusNodeId);
	if (retVal != UA_STATUSCODE_GOOD)
	{
		return retVal;
	}

	// Get the status value
	UA_Variant outValue;
	retVal = UA_Server_readValue(server, statusNodeId, &outValue);
	// Check if already started
	UA_Boolean status = *(UA_Boolean *)outValue.data;
	if (status)
	{
		return UA_STATUSCODE_BADNOTSUPPORTED;
	}

	// Set the status value
	status = true;
	UA_Variant value;
	UA_Variant_setScalar(&value, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
	retVal = UA_Server_writeValue(server, statusNodeId, value);
	
	return retVal;
}

static UA_StatusCode pumpStopPumpMethod(
	UA_Server        *server,
	const UA_NodeId  *sessionId,
	void             *sessionHandle,
	const UA_NodeId  *methodId,
	void             *methodContext,
	const UA_NodeId  *objectId,
	void             *objectContext,
	size_t            inputSize,
	const UA_Variant *input,
	size_t            outputSize,
	UA_Variant       *output)
{
	const int * p_intObjContext = (const int *)objectContext;
	qDebug() << "Stop Pump method called with object context : " << *p_intObjContext;

	UA_NodeId statusNodeId;
	UA_StatusCode retVal = findNodeIdComponentByQualifiedName(server,
		objectId,
		&UA_QUALIFIEDNAME(1, (char*)"Status"),
		&statusNodeId);
	if (retVal != UA_STATUSCODE_GOOD)
	{
		return retVal;
	}

	// Get the status value
	UA_Variant outValue;
	retVal = UA_Server_readValue(server, statusNodeId, &outValue);
	// Check if already started
	UA_Boolean status = *(UA_Boolean *)outValue.data;
	if (!status)
	{
		return UA_STATUSCODE_BADNOTSUPPORTED;
	}

	// Set the status value
	status = false;
	UA_Variant value;
	UA_Variant_setScalar(&value, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
	retVal = UA_Server_writeValue(server, statusNodeId, value);

	return retVal;
}

static UA_StatusCode pumpIncRpmMethod(
	UA_Server        *server,
	const UA_NodeId  *sessionId,
	void             *sessionHandle,
	const UA_NodeId  *methodId,
	void             *methodContext,
	const UA_NodeId  *objectId,
	void             *objectContext,
	size_t            inputSize,
	const UA_Variant *input,
	size_t            outputSize,
	UA_Variant       *output)
{
	const int * p_intObjContext = (const int *)objectContext;
	qDebug() << "Increase RPM method called with object context : " << *p_intObjContext;

	UA_NodeId rpmNodeId;
	UA_StatusCode retVal = findNodeIdComponentByQualifiedName(server,
		objectId,
		&UA_QUALIFIEDNAME(1, (char*)"MotorRPMs"),
		&rpmNodeId);
	if (retVal != UA_STATUSCODE_GOOD)
	{
		return retVal;
	}

	// Get the current rpm value
	UA_Variant outValue;
	retVal = UA_Server_readValue(server, rpmNodeId, &outValue); // TODO : possible memory leak!
	UA_Double rpm = *(UA_Double *)outValue.data;

	// Get the delta rpm value
	UA_Double delta_rpm = *(UA_Double *)input->data;

	// Set the rpm value
	rpm = rpm + delta_rpm;
	UA_Variant value;
	UA_Variant_setScalar(&value, &rpm, &UA_TYPES[UA_TYPES_DOUBLE]);
	retVal = UA_Server_writeValue(server, rpmNodeId, value);

	// Set the method output
	UA_Variant_init(output);
	output->type               = &UA_TYPES[UA_TYPES_DOUBLE];
	output->arrayLength        = 0;
	output->data               = UA_malloc(UA_TYPES[UA_TYPES_DOUBLE].memSize);
	*(UA_Double *)output->data = rpm;

	return retVal;
}


// Create Object Types
static void defineObjectTypes(UA_Server *server) 
{
	// Define (abstract) ObjectType "Device"
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
	// Define "ManufacturerName" Component (Variable) for Device
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
	// Define "ManufacturerName" mandatory by creating a special Reference from the Property to a special node called MODELLINGRULE_MANDATORY
	UA_Server_addReference(server, 
		                   manufacturerNameId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);
	// Define "ModelName" Component (Variable) for Device
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

	// Define ObjectType "PumpType" that inherits from "Device" (using the Reference HASSUBTYPE)
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
	// Define "Status" Component (Variable) for PumpType
	UA_VariableAttributes statusAttr = UA_VariableAttributes_default;
	statusAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Status");
	statusAttr.valueRank   = UA_VALUERANK_SCALAR;
	statusAttr.dataType    = UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN);
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
	// Define "Status" mandatory
	UA_Server_addReference(server, 
		                   statusId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);
	// Define "MotorRPM" Component (Variable) for PumpType
	UA_VariableAttributes rpmAttr = UA_VariableAttributes_default;
	rpmAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"MotorRPM");
	rpmAttr.valueRank   = UA_VALUERANK_SCALAR;
	rpmAttr.dataType    = UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE);
	UA_NodeId rpmId;
	UA_Server_addVariableNode(server, 
		                      UA_NODEID_NULL, 
		                      pumpTypeId,
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		                      UA_QUALIFIEDNAME(1, (char*)"MotorRPMs"),
		                      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), 
		                      rpmAttr, 
		                      NULL, 
		                      &rpmId);
	// Define "MotorRPM" mandatory
	UA_Server_addReference(server, 
		                   rpmId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);
	// Add Constructor for PumpType
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = pumpTypeConstructor;
	lifecycle.destructor  = pumpTypeDestructor;
	UA_Server_setNodeTypeLifecycle(server, pumpTypeId, lifecycle);

	// Add "StartPump" method
	UA_MethodAttributes startAttrs = UA_MethodAttributes_default;
	startAttrs.description    = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Starts the pump, setting the `Status` variable to true");
	startAttrs.displayName    = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Start Pump");
	startAttrs.executable     = true;
	startAttrs.userExecutable = true;
	UA_NodeId methodStartPumpId;
	UA_Server_addMethodNode(                           
		server,                                             // UA_Server                 *server, 
		UA_NODEID_NULL,                                     // const UA_NodeId            requestedNewNodeId,
		pumpTypeId,                                         // const UA_NodeId            parentNodeId, 
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT), // const UA_NodeId            referenceTypeId, (or ?UA_NS0ID_HASORDEREDCOMPONENT)
		UA_QUALIFIEDNAME (1, (char*)"start pump"),          // const UA_QualifiedName     browseName, 
		startAttrs,                                         // const UA_MethodAttributes  attr,
		&pumpStartPumpMethod,                               // UA_MethodCallback          method,
		0,                                                  // size_t                     inputArgumentsSize, 
		NULL,                                               // const UA_Argument         *inputArguments,
		0,                                                  // size_t                     outputArgumentsSize, 
		NULL,                                               // const UA_Argument         *outputArguments,
		NULL,                                               // void                      *nodeContext, 
		&methodStartPumpId                                  // UA_NodeId                 *outNewNodeId
	);
	// Define "StartPump" method mandatory
	UA_Server_addReference(server, 
		                   methodStartPumpId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);

	// Add "StopPump" method
	UA_MethodAttributes stopAttrs = UA_MethodAttributes_default;
	stopAttrs.description    = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Stops the pump, setting the `Status` variable to true");
	stopAttrs.displayName    = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Stop Pump");
	stopAttrs.executable     = true;
	stopAttrs.userExecutable = true;
	UA_NodeId methodStopPumpId;
	UA_Server_addMethodNode(                         
		server,                                      
		UA_NODEID_NULL,                              
		pumpTypeId,                                  
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT),
		UA_QUALIFIEDNAME (1, (char*)"stop pump"),
		stopAttrs,                                   
		&pumpStopPumpMethod,
		0,                                           
		NULL,                                        
		0,                                           
		NULL,                                        
		NULL,                                        
		&methodStopPumpId                            
	);
	// Define "StopPump" method mandatory
	UA_Server_addReference(server, 
		                   methodStopPumpId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);

	/* "pumpIncRpmMethod" method input argument */
    UA_Argument inputArgument;
    UA_Argument_init(&inputArgument);
    inputArgument.description         = UA_LOCALIZEDTEXT("en-US", "RPMs to be added to the current value [double]");
    inputArgument.name                = UA_STRING("delta RPMs");
	inputArgument.valueRank           = UA_VALUERANK_SCALAR;
    inputArgument.dataType            = UA_TYPES[UA_TYPES_DOUBLE].typeId;
	/* "pumpIncRpmMethod" method output argument */
	UA_Argument outputArgument;
	UA_Argument_init(&outputArgument);
	outputArgument.description         = UA_LOCALIZEDTEXT("en-US", "RPMs result after adding the delta input to the current value [double]");
	outputArgument.name                = UA_STRING("result RPMs");
	outputArgument.valueRank           = UA_VALUERANK_SCALAR;
	outputArgument.dataType            = UA_TYPES[UA_TYPES_DOUBLE].typeId;
	// Add "pumpIncRpmMethod" method
	UA_MethodAttributes rpmAttrs   = UA_MethodAttributes_default;
	rpmAttrs.description           = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Increases the Rpms in the pump, modifing the `MotorRPM` variable");
	rpmAttrs.displayName           = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Increase RPMs");
	rpmAttrs.executable            = true;
	rpmAttrs.userExecutable        = true;
	UA_NodeId incRpmPumpId         = UA_NODEID_NULL;
	incRpmPumpId.namespaceIndex    = 1;
	incRpmPumpId.identifierType    = UA_NODEIDTYPE_STRING;
	incRpmPumpId.identifier.string = UA_BYTESTRING_ALLOC((char*)"incrpms");
	UA_Server_addMethodNode(                         
		server,                                      
		incRpmPumpId,
		pumpTypeId,                                  
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT),
		UA_QUALIFIEDNAME (1, (char*)"incrpms"),
		rpmAttrs,
		&pumpIncRpmMethod,
		1,                                           
		&inputArgument,
		1,                                           
		&outputArgument,
		NULL,                                        
		NULL
	);
	// Define "pumpIncRpmMethod" method mandatory
	UA_Server_addReference(server, 
		                   incRpmPumpId,
		                   UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                   UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                   true);

}

static void addPumpObjectInstance(UA_Server *server, char *name, char *nodeid) 
{
	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
	oAttr.displayName    = UA_LOCALIZEDTEXT((char*)"en-US", name);
	UA_NodeId pumpNodeId = UA_NODEID_NULL;
	if (nodeid)
	{
		pumpNodeId.namespaceIndex    = 1;
		pumpNodeId.identifierType    = UA_NODEIDTYPE_STRING;
		pumpNodeId.identifier.string = UA_BYTESTRING_ALLOC(nodeid);
	}
	UA_Server_addObjectNode(server, 
		                    pumpNodeId,
		                    UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
		                    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		                    UA_QUALIFIEDNAME(1, name),
		                    pumpTypeId, /* this refers to the object type identifier */
		                    oAttr, 
		                    NULL,    // void *nodeContext
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
			addPumpObjectInstance(server, (char*)"pump1", (char*)"pump1");
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
