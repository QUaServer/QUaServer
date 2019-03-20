#include <signal.h>
#include <stdio.h>
#include "open62541.h"

static UA_NodeId addScalarVariable(UA_Server* server, char * varName) {

	UA_VariableAttributes vattr = UA_VariableAttributes_default;

	vattr.description = UA_LOCALIZEDTEXT("", varName);
	vattr.displayName = UA_LOCALIZEDTEXT("", varName);
	vattr.dataType    = UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
	vattr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	vattr.valueRank   = UA_VALUERANK_ANY;
	UA_Int32 test = 1;
	UA_Variant_setScalar(&vattr.value, &test, &UA_TYPES[UA_TYPES_INT32]);

	// [NOTE] : doing this makes variable a scalar PERMANENTLY
	//vattr.valueRank = UA_VALUERANK_SCALAR;

	UA_NodeId outNodeId;
	UA_StatusCode st = UA_Server_addVariableNode(server,
		UA_NODEID_STRING (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),       
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),           
		UA_QUALIFIEDNAME (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		vattr,                                              
		NULL,                                               
		&outNodeId);
	assert(st == UA_STATUSCODE_GOOD);

	return outNodeId;
}

static UA_NodeId addArrayVariable(UA_Server* server, char * varName) {

	UA_VariableAttributes vattr = UA_VariableAttributes_default;

	vattr.description = UA_LOCALIZEDTEXT("", varName);
	vattr.displayName = UA_LOCALIZEDTEXT("", varName);
	vattr.dataType    = UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
	vattr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

	// [NOTE] : doing this makes variable an array PERMANENTLY
	//vattr.valueRank   = UA_VALUERANK_ONE_DIMENSION;
	//UA_UInt32 arrayDims[1]    = { 3 };
	//vattr.arrayDimensions     = arrayDims;
	//vattr.arrayDimensionsSize = 1;
	
	UA_Int32 arrayVals[3] = { 1, 2, 3 };
	UA_Variant_setArrayCopy(&vattr.value, arrayVals, 3, &UA_TYPES[UA_TYPES_INT32]);

	UA_UInt32 arrayDims[1] = { 3 };
	vattr.value.arrayDimensions     = arrayDims;
	vattr.value.arrayDimensionsSize = 1;
	vattr.valueRank                 = UA_VALUERANK_ANY;
	
	UA_NodeId outNodeId;
	UA_StatusCode st = UA_Server_addVariableNode(server,
		UA_NODEID_STRING (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),       
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),           
		UA_QUALIFIEDNAME (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		vattr,                                              
		NULL,                                               
		&outNodeId);
	assert(st == UA_STATUSCODE_GOOD);

	return outNodeId;
}

// [NOTE] : only works if UA_VALUERANK_ANY
static void scalarToArray(UA_Server* server, UA_NodeId * scalarNodeId)
{
	UA_Variant varArray;
	UA_Int32 arrayVals[3] = { 4, 5, 6 };
	UA_Variant_setArrayCopy(&varArray, arrayVals, 3, &UA_TYPES[UA_TYPES_INT32]);

	UA_UInt32 arrayDims[1]       = { 3 };
	varArray.arrayDimensions     = arrayDims;
	varArray.arrayDimensionsSize = 1;

	UA_StatusCode st = UA_Server_writeValue(server, *scalarNodeId, varArray);
	assert(st == UA_STATUSCODE_GOOD);
}

// [NOTE] : only works if UA_VALUERANK_ANY
static void arrayToScalar(UA_Server* server, UA_NodeId * arrayNodeId)
{
	UA_Variant varScalar;
	UA_Int32 test = 9;
	UA_Variant_setScalar(&varScalar, &test, &UA_TYPES[UA_TYPES_INT32]);

	UA_StatusCode st = UA_Server_writeValue(server, *arrayNodeId, varScalar);
	assert(st == UA_STATUSCODE_GOOD);
}

UA_Boolean running = true;
static void stopHandler(int sig) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
	running = false;
}

int main(void) {
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);

	UA_ServerConfig* config = UA_ServerConfig_new_default();

	UA_Server* server = UA_Server_new(config);

	UA_NodeId scalarNodeId = addScalarVariable(server, "Foo");
	UA_NodeId arrayNodeId  = addArrayVariable (server, "Bar");

	scalarToArray(server, &scalarNodeId);
	arrayToScalar(server, &arrayNodeId);

	UA_StatusCode retval = UA_Server_run(server, &running);
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);
	return (int)retval;
}