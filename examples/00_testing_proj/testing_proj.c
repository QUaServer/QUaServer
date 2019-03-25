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
	// NOTE : Works OK if nodeId UA_NODEID_NULL or UA_NODEID_NUMERIC(1, 666)
	//        or if we comment out this function's body and only return UA_STATUSCODE_GOOD
	
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	//bDesc->nodeId          = *nodeId; // NOPE !
	UA_NodeId_copy(nodeId, &bDesc->nodeId); // from child
	bDesc->browseDirection = UA_BROWSEDIRECTION_INVERSE; //  look upwards
	bDesc->includeSubtypes = true;
	bDesc->resultMask      = UA_BROWSERESULTMASK_BROWSENAME | UA_BROWSERESULTMASK_DISPLAYNAME;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(server, 0, bDesc);
	assert(bRes.statusCode == UA_STATUSCODE_GOOD);

	// do something with results ...

	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);

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
static void addBaseObject(UA_Server* server, char * varName) {

	UA_ObjectAttributes oattr = UA_ObjectAttributes_default;

	oattr.description = UA_LOCALIZEDTEXT("", varName);
	oattr.displayName = UA_LOCALIZEDTEXT("", varName);

	UA_StatusCode st = UA_Server_addObjectNode(
		server,
		UA_NODEID_STRING (1, varName), // NOTE : Works OK with : UA_NODEID_NULL or UA_NODEID_NUMERIC(1, 666)
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
	addBaseObject(server, "Foo");

	UA_StatusCode retval = UA_Server_run(server, &running);
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);
	return (int)retval;
}