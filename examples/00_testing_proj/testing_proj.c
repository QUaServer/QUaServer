#include <assert.h>
#include "open62541.h"

int main(void) 
{
    // create server
    UA_ServerConfig *config = UA_ServerConfig_new_default();
    UA_Server       *server = UA_Server_new(config);
 
    // add base object instance
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    UA_NodeId instanceId;
    UA_Server_addObjectNode(server, 
                            UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME (1, "Instance"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttr,              
                            NULL,
                            &instanceId);


	UA_BrowseDescription * bDesc = UA_BrowseDescription_new();
	bDesc->nodeId          = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); // instanceId;
	bDesc->browseDirection = UA_BROWSEDIRECTION_BOTH; //  UA_BROWSEDIRECTION_FORWARD; // UA_BROWSEDIRECTION_INVERSE;
	//bDesc->referenceTypeId = NONE!
	bDesc->includeSubtypes = false;
	bDesc->nodeClassMask   = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE; //UA_BROWSERESULTMASK_ALL;
	bDesc->resultMask      = UA_BROWSERESULTMASK_ALL;

	UA_BrowseResult bRes = UA_Server_browse(server, 0, bDesc);
	assert(bRes.statusCode == UA_STATUSCODE_GOOD);
	while (bRes.referencesSize > 0)
	{
		for (size_t i = 0; i < bRes.referencesSize; i++)
		{
			UA_ReferenceDescription rDesc = bRes.references[i];
			UA_NodeId nodeId = rDesc.nodeId.nodeId;
			/*
                UA_NodeId         referenceTypeId;
                UA_Boolean        isForward;
                UA_ExpandedNodeId nodeId;
                UA_QualifiedName  browseName;
                UA_LocalizedText  displayName;
                UA_NodeClass      nodeClass;
                UA_ExpandedNodeId typeDefinition;
			*/
			printf("%-16.*s\n", (int)rDesc.browseName.name.length, rDesc.browseName.name.data);
		}
		bRes = UA_Server_browseNext(server, true, &bRes.continuationPoint);
	}
	// cleanup
	UA_BrowseDescription_deleteMembers(bDesc);
	UA_BrowseDescription_delete(bDesc);
	UA_BrowseResult_deleteMembers(&bRes);


    return 0;
}