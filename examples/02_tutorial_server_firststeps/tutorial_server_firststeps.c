#include <assert.h>
#include "open62541.h"

int main(void) 
{
	UA_NodeId nodeId1;
	nodeId1.namespaceIndex     = 1;
	nodeId1.identifierType     = UA_NODEIDTYPE_NUMERIC;
	nodeId1.identifier.numeric = 319;

	UA_UInt32 hashNodeId1 = UA_NodeId_hash(&nodeId1);

	UA_NodeId nodeId2;
	nodeId2.namespaceIndex     = 1;
	nodeId2.identifierType     = UA_NODEIDTYPE_NUMERIC;
	nodeId2.identifier.numeric = 320; // DIFFERENT !

	UA_UInt32 hashNodeId2 = UA_NodeId_hash(&nodeId2);

	// FAILS !
	assert(hashNodeId1 != hashNodeId2);

	return;
}