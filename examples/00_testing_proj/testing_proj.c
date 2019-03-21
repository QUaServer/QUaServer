#include <signal.h>
#include <stdio.h>
#include "open62541.h"

static UA_StatusCode genericConstructor(UA_Server       * server, 
	                                    const UA_NodeId * sessionId, 
	                                    void            * sessionContext, 
	                                    const UA_NodeId * typeNodeId, 
	                                    void            * typeNodeContext, 
	                                    const UA_NodeId * nodeId, 
	                                    void            ** nodeContext)
{
	return UA_STATUSCODE_GOOD;
}

static void setBaseObjectTypeConstructor(UA_Server *server)
{
	// set context
	UA_StatusCode st = UA_Server_setNodeContext(server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), (void*)"BaseObjectType");
	assert(st == UA_STATUSCODE_GOOD);
	// set constructor
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &genericConstructor;
	lifecycle.destructor = NULL;
	st = UA_Server_setNodeTypeLifecycle(server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), lifecycle);
	assert(st == UA_STATUSCODE_GOOD);
}

static UA_NodeId objNodeId;
static void addFooObject(UA_Server* server, char * varName) {

	UA_ObjectAttributes oattr = UA_ObjectAttributes_default;

	oattr.description = UA_LOCALIZEDTEXT("", varName);
	oattr.displayName = UA_LOCALIZEDTEXT("", varName);

	UA_StatusCode st = UA_Server_addObjectNode(
		server,
		UA_NODEID_STRING (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),       
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		UA_QUALIFIEDNAME (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
		oattr,                                              
		NULL,                                               
		&objNodeId
	);
	assert(st == UA_STATUSCODE_GOOD);
}

static UA_StatusCode methodCallback(UA_Server        * server,
	                                const UA_NodeId  * sessionId,
	                                void             * sessionContext,
	                                const UA_NodeId  * methodId,
	                                void             * methodContext,
	                                const UA_NodeId  * objectId,
	                                void             * objectContext,
	                                size_t             inputSize,
	                                const UA_Variant * input,
	                                size_t             outputSize,
	                                UA_Variant       * output)
{
	UA_StatusCode st = UA_Server_deleteNode(server, objNodeId, true);
	return st;
}

static void addDeleteFooMethod(UA_Server* server, char * varName) {

	UA_MethodAttributes methAttr = UA_MethodAttributes_default;
    methAttr.executable     = true;
    methAttr.userExecutable = true;
    methAttr.description    = UA_LOCALIZEDTEXT("", varName);
    methAttr.displayName    = UA_LOCALIZEDTEXT("", varName);
    // create callback
	UA_StatusCode st = UA_Server_addMethodNode(
		server,
		UA_NODEID_NULL,
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),    
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME (1, varName),
		methAttr,
		&methodCallback,
		0,
		NULL,
		0,
		NULL,
		NULL,
		NULL
	);
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

	setBaseObjectTypeConstructor(server);
	addFooObject(server, "Foo");
	addDeleteFooMethod(server, "deleteFoo");

	UA_StatusCode retval = UA_Server_run(server, &running);
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);
	return (int)retval;
}