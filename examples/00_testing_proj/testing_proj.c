#include <assert.h>
#include <signal.h>
#include "open62541.h"

static void printNodeId(const UA_NodeId nodeId)
{
	printf("ns=%i;", nodeId.namespaceIndex);
	switch (nodeId.identifierType)
	{
	case UA_NODEIDTYPE_NUMERIC:
		printf("i=%i", nodeId.identifier.numeric);
		break;
	case UA_NODEIDTYPE_STRING:
		printf("s=%s", nodeId.identifier.string.data);
		break;
	case UA_NODEIDTYPE_GUID:
		printf("g=%i%i%i&i", nodeId.identifier.guid.data1, nodeId.identifier.guid.data2, nodeId.identifier.guid.data3, nodeId.identifier.guid.data4);
		break;
	case UA_NODEIDTYPE_BYTESTRING:
		printf("b=%s", nodeId.identifier.byteString.data);
		break;
	}
}

static UA_NodeId getParentNodeId(const UA_NodeId childNodeId, UA_Server * server)
{
	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	bDesc->nodeId          = childNodeId; // from child
	bDesc->browseDirection = UA_BROWSEDIRECTION_INVERSE; //  look upwards
	bDesc->includeSubtypes = true;
	bDesc->resultMask      = UA_BROWSERESULTMASK_BROWSENAME | UA_BROWSERESULTMASK_DISPLAYNAME;
	// browse
	UA_BrowseResult bRes = UA_Server_browse(server, 0, bDesc);
	assert(bRes.statusCode == UA_STATUSCODE_GOOD);
	UA_NodeId nodeId = UA_NODEID_NULL;
	size_t j = 0;
	while (bRes.referencesSize > 0)
	{
		assert(j == 0); // assert only one parent
		for (size_t i = 0; i < bRes.referencesSize; i++)
		{
			assert(i == 0); // assert only one parent
			UA_ReferenceDescription rDesc = bRes.references[i];
			nodeId = rDesc.nodeId.nodeId;			
		}
		bRes = UA_Server_browseNext(server, true, &bRes.continuationPoint);
		j++;
	}
	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);
	// return
	return nodeId;
}

static int contructorCount = 0;
static UA_StatusCode genericConstructor(UA_Server       * server, 
	                                    const UA_NodeId * sessionId, 
	                                    void            * sessionContext, 
	                                    const UA_NodeId * typeNodeId, 
	                                    void            * typeNodeContext, 
	                                    const UA_NodeId * nodeId, 
	                                    void            ** nodeContext)
{
	const char * typeName = typeNodeContext;
	printf("%i) [", ++contructorCount);
	printNodeId(*nodeId);
	printf("] ");
	printf("%s Constructor\n>> Parents", typeName);
	// print parents
	UA_NodeId objectsNodeId         = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	UA_NodeId baseDataVarTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
	UA_NodeId baseObjectTypeNodeId  = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
	UA_NodeId parentNodeId = getParentNodeId(*nodeId, server);
	while (!UA_NodeId_equal(&parentNodeId, &objectsNodeId) &&
		   !UA_NodeId_equal(&parentNodeId, &baseDataVarTypeNodeId) &&
		   !UA_NodeId_equal(&parentNodeId, &baseObjectTypeNodeId) &&
		   !UA_NodeId_isNull(&parentNodeId))
	{
		printf(" -> [");
		printNodeId(parentNodeId);
		printf("]");
		parentNodeId = getParentNodeId(parentNodeId, server);
	}
	// print final parent
	if (UA_NodeId_isNull(&parentNodeId))
	{
		printf(" -> [UA_NODEID_NULL]");
	}
	else if (UA_NodeId_equal(&parentNodeId, &objectsNodeId))
	{
		printf(" -> [UA_NS0ID_OBJECTSFOLDER]");
	}
	else if (UA_NodeId_equal(&parentNodeId, &baseDataVarTypeNodeId))
	{
		printf(" -> [UA_NS0ID_BASEDATAVARIABLETYPE]");
	}
	else if (UA_NodeId_equal(&parentNodeId, &baseObjectTypeNodeId))
	{
		printf(" -> [UA_NS0ID_BASEOBJECTTYPE]");
	}
	printf("\n\n");
	return UA_STATUSCODE_GOOD;
}

static void setBaseDataVariableTypeConstructor(UA_Server *server)
{
	// set context
	UA_StatusCode st = UA_Server_setNodeContext(server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), (void*)"BaseDataVariableType");
	assert(st == UA_STATUSCODE_GOOD);
	// set constructor
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &genericConstructor;
	st = UA_Server_setNodeTypeLifecycle(server, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), lifecycle);
	assert(st == UA_STATUSCODE_GOOD);
}

static UA_NodeId myVariableTypeNodeId;
static void addMyVariableType(UA_Server *server) 
{
	// create variable type attributes
	UA_VariableTypeAttributes vtAttr = UA_VariableTypeAttributes_default;	  
	vtAttr.displayName               = UA_LOCALIZEDTEXT("en-US", "MyVariableType");
	// add new variable type
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name = UA_STRING_ALLOC("MyVariableType");
	UA_StatusCode st = UA_Server_addVariableTypeNode(server,
			                                         UA_NODEID_NULL,                            // requested nodeId
			                                         UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), // parent (variable type)
			                                         UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), // parent relation with child
			                                         browseName,
			                                         UA_NODEID_NULL,                            // typeDefinition ??
			                                         vtAttr,
			                                         (void*)"MyVariableType",                   // context  
			                                         &myVariableTypeNodeId);                    // new variable type id
	assert(st == UA_STATUSCODE_GOOD);
	// add constructor
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &genericConstructor;
	st = UA_Server_setNodeTypeLifecycle(server, myVariableTypeNodeId, lifecycle);
	assert(st == UA_STATUSCODE_GOOD);
	// add base data variable as child
	UA_VariableAttributes vAttr = UA_VariableAttributes_default;
	vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "BaseDataVariableChild");
	// add variable
	UA_NodeId childNodeId;
	st = UA_Server_addVariableNode(server,
								   UA_NODEID_NULL,                                      // requested nodeId
								   myVariableTypeNodeId,                                // parent
								   UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),         // parent relation with child
								   browseName,		  
								   UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), // type id
								   vAttr, 			  
								   (void *)"BaseDataVariableChild",                     // context
								   &childNodeId);                                       // output nodeId to make mandatory
	assert(st == UA_STATUSCODE_GOOD);
	// make mandatory child
	st = UA_Server_addReference(server,
		                        childNodeId,
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                        true);
	assert(st == UA_STATUSCODE_GOOD);
}

static UA_NodeId myVariableOtherTypeNodeId;
static void addMyVariableOtherType(UA_Server *server) 
{
	// create variable type attributes
	UA_VariableTypeAttributes vtAttr = UA_VariableTypeAttributes_default;	  
	vtAttr.displayName               = UA_LOCALIZEDTEXT("en-US", "MyVariableOtherType");
	// add new variable type
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name = UA_STRING_ALLOC("MyVariableOtherType");
	UA_StatusCode st = UA_Server_addVariableTypeNode(server,
			                                         UA_NODEID_NULL,                            // requested nodeId
		                                             UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), // parent (variable type)
			                                         UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), // parent relation with child
			                                         browseName,
			                                         UA_NODEID_NULL,                            // typeDefinition ??
			                                         vtAttr,
			                                         (void*)"MyVariableOtherType",              // context  
			                                         &myVariableOtherTypeNodeId);               // new variable type id
	assert(st == UA_STATUSCODE_GOOD);
	// add constructor
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &genericConstructor;
	st = UA_Server_setNodeTypeLifecycle(server, myVariableOtherTypeNodeId, lifecycle);
	assert(st == UA_STATUSCODE_GOOD);
	// add base data variable as child
	UA_VariableAttributes vAttr = UA_VariableAttributes_default;
	vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MyVariableAsComponent");
	// add variable
	UA_NodeId childNodeId;
	st = UA_Server_addVariableNode(server,
								   UA_NODEID_NULL,                              // requested nodeId
								   myVariableOtherTypeNodeId,                   // parent
								   UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), // parent relation with child
								   browseName,		  						    
								   myVariableTypeNodeId,                        // type id
								   vAttr, 			  						    
								   (void *)"MyVariableAsComponent",                // context
								   &childNodeId);                               // output nodeId to make mandatory
	assert(st == UA_STATUSCODE_GOOD);
	// make mandatory child
	st = UA_Server_addReference(server,
		                        childNodeId,
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                        true);
	assert(st == UA_STATUSCODE_GOOD);
}

static UA_NodeId myObjectTypeNodeId;
static void addMyObjectType(UA_Server *server) 
{
	// create variable type attributes
	UA_ObjectTypeAttributes otAttr = UA_ObjectTypeAttributes_default;
	otAttr.displayName             = UA_LOCALIZEDTEXT("en-US", "MyObjectType");
	// add new variable type
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name = UA_STRING_ALLOC("MyObjectType");
	UA_StatusCode st = UA_Server_addObjectTypeNode(server,
			                                       UA_NODEID_NULL,                                // requested nodeId
			                                       UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), // parent (variable type)
			                                       UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),     // parent relation with child
			                                       browseName,
			                                       otAttr,
			                                       (void*)"MyObjectType",                         // context 
			                                       &myObjectTypeNodeId);                          // new variable type id
	assert(st == UA_STATUSCODE_GOOD);
	// add constructor
	UA_NodeTypeLifecycle lifecycle;
	lifecycle.constructor = &genericConstructor;
	st = UA_Server_setNodeTypeLifecycle(server, myObjectTypeNodeId, lifecycle);
	assert(st == UA_STATUSCODE_GOOD);
	// add sub type as child
	UA_VariableAttributes vAttr = UA_VariableAttributes_default;
	vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MyVariableOtherType");
	// add variable
	UA_NodeId childNodeId;
	st = UA_Server_addVariableNode(server,
								   UA_NODEID_NULL,                                      // requested nodeId
								   myObjectTypeNodeId,                                  // parent
								   UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),         // parent relation with child
								   browseName,		  
		                           myVariableOtherTypeNodeId,                           // type id
								   vAttr, 			  
								   (void *)"MyVariableOtherType",                       // context
								   &childNodeId);                                       // output nodeId to make mandatory
	assert(st == UA_STATUSCODE_GOOD);
	// make mandatory child
	st = UA_Server_addReference(server,
		                        childNodeId,
		                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
		                        UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
		                        true);
	assert(st == UA_STATUSCODE_GOOD);
}

static UA_NodeId addMyObjectInstance(const UA_NodeId parentNodeId, UA_Server * server)
{
	UA_NodeId myObjectInstanceNodeId;
	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
	oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MyObjectInstance");
	// add variable
	UA_QualifiedName browseName;
	browseName.namespaceIndex = 1;
	browseName.name = UA_STRING_ALLOC("MyObjectInstance");
	auto st = UA_Server_addObjectNode(server,
                                      UA_NODEID_NULL,                              // requested nodeId
                                      parentNodeId,                                // parent
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), // parent relation with child
                                      browseName,
                                      myObjectTypeNodeId, 
                                      oAttr, 
                                      (void*)"MyObjectInstance",                   // context
                                      &myObjectInstanceNodeId);                    // new variable instance id
	assert(st == UA_STATUSCODE_GOOD);
	return myObjectInstanceNodeId;
}

UA_Boolean running = true;
static void stopHandler(int sign) 
{
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
	running = false;
}


int main(void) 
{

	UA_ServerConfig *config = UA_ServerConfig_new_default();
	UA_Server       *server = UA_Server_new(config);

	UA_NodeId objectsNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

	setBaseDataVariableTypeConstructor(server);

	addMyVariableType(server);
	addMyVariableOtherType(server);

	addMyObjectType(server);
	addMyObjectInstance(objectsNodeId, server);


	UA_StatusCode retval = UA_Server_run(server, &running);
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);
	return (int)retval;
}