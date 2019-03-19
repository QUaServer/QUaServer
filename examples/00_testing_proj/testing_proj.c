#include <signal.h>
#include <stdio.h>
#include "open62541.h"

#define UA_NS0ID_DEFAULTBINARY 8251

typedef struct
{
	UA_Int64         Value;
	UA_LocalizedText DisplayName;
	UA_LocalizedText Description;
} EnumValue;

static UA_StatusCode createEnumValue(const EnumValue * enumVal, UA_ExtensionObject * outExtObj)
{
	if (!outExtObj)
	{
		return UA_STATUSCODE_BADOUTOFMEMORY;
	}
	outExtObj->encoding               = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
	outExtObj->content.encoded.typeId = UA_NODEID_NUMERIC(0, UA_NS0ID_DEFAULTBINARY);
	UA_ByteString_allocBuffer(&outExtObj->content.encoded.body, 65000);
	UA_Byte * p_posExtObj = outExtObj->content.encoded.body.data;
	const UA_Byte * p_endExtObj = &outExtObj->content.encoded.body.data[65000];
	// encode enum value
	UA_StatusCode st = UA_STATUSCODE_GOOD;
	st |= UA_encodeBinary(&enumVal->Value      , &UA_TYPES[UA_TYPES_INT64]        , &p_posExtObj, &p_endExtObj, NULL, NULL);
	st |= UA_encodeBinary(&enumVal->DisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &p_posExtObj, &p_endExtObj, NULL, NULL);
	st |= UA_encodeBinary(&enumVal->Description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &p_posExtObj, &p_endExtObj, NULL, NULL);
	if (st != UA_STATUSCODE_GOOD)
	{
		return st;
	}
	size_t p_extobj_encOffset = (uintptr_t)(p_posExtObj - outExtObj->content.encoded.body.data);
	outExtObj->content.encoded.body.length = p_extobj_encOffset;
	UA_Byte * p_extobj_newBody = (UA_Byte *)UA_malloc(p_extobj_encOffset);
	if (!p_extobj_newBody)
	{
		return UA_STATUSCODE_BADOUTOFMEMORY;
	}
	memcpy(p_extobj_newBody, outExtObj->content.encoded.body.data, p_extobj_encOffset);
	UA_Byte * p_extobj_madatory_oldBody  = outExtObj->content.encoded.body.data;
	outExtObj->content.encoded.body.data = p_extobj_newBody;
	UA_free(p_extobj_madatory_oldBody);
	// success
	return UA_STATUSCODE_GOOD;
}

static UA_StatusCode addEnumValues(UA_Server * server, UA_NodeId * parent, const UA_UInt32 numEnumValues, const EnumValue * enumValues)
{
	// setup variable attrs
	UA_StatusCode retVal         = UA_STATUSCODE_GOOD;
	UA_VariableAttributes attr   = UA_VariableAttributes_default;
	attr.minimumSamplingInterval = 0.000000;
	attr.userAccessLevel         = 1;
	attr.accessLevel             = 1;
	attr.valueRank               = 1;
	attr.arrayDimensionsSize     = 1;
	attr.arrayDimensions         = (UA_UInt32 *)UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]);
	if (!attr.arrayDimensions)
	{
		return UA_STATUSCODE_BADOUTOFMEMORY;
	}
	attr.arrayDimensions[0] = 0;
	attr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMVALUETYPE);
	// create array of enum values
	UA_ExtensionObject * arr_extobjs = (UA_ExtensionObject *)UA_malloc(sizeof(UA_ExtensionObject) * numEnumValues);
	for (size_t i = 0; i < numEnumValues; i++)
	{
		retVal |= createEnumValue(&enumValues[i], &arr_extobjs[i]);
		assert(retVal == UA_STATUSCODE_GOOD);
	}
	// create variant with array of enum values
	UA_Variant_setArray(&attr.value, arr_extobjs, (UA_Int32)numEnumValues, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
	attr.displayName   = UA_LOCALIZEDTEXT("", "EnumValues");
	attr.description   = UA_LOCALIZEDTEXT("", "");
	attr.writeMask     = 0;
	attr.userWriteMask = 0;
	// add variable with array of enum values
	UA_NodeId enumValuesNodeId;
	retVal |= UA_Server_addVariableNode(server,
                                        UA_NODEID_NULL, 
                                        *parent, 
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                        UA_QUALIFIEDNAME (0, "EnumValues"), 
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), 
                                        attr, 
                                        NULL, 
                                        &enumValuesNodeId);
	assert(retVal == UA_STATUSCODE_GOOD);
	// cleanup
	UA_Array_delete(attr.arrayDimensions, 1, &UA_TYPES[UA_TYPES_UINT32]);
	for (size_t i = 0; i < numEnumValues; i++)
	{
		UA_ExtensionObject_deleteMembers(&arr_extobjs[i]);
	}
	UA_free(arr_extobjs);
	// make mandatory
	retVal |= UA_Server_addReference(server,
					                 enumValuesNodeId,
					                 UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
					                 UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), 
					                 true);
	assert(retVal == UA_STATUSCODE_GOOD);
	// success
	return retVal;
}

static UA_NodeId customEnumTypeNodeId;
static void addCustomEnumType(UA_Server* server, char * enumName) 
{
	// setup new data type attrs
	UA_DataTypeAttributes ddaatt = UA_DataTypeAttributes_default;
	ddaatt.description = UA_LOCALIZEDTEXT("", enumName);
	ddaatt.displayName = UA_LOCALIZEDTEXT("", enumName);
	// add new data type (enum)
	UA_StatusCode st = UA_Server_addDataTypeNode(
		server,
		UA_NODEID_STRING (1, enumName), // [IMPORTANT] with UA_NODEID_NULL fails, must be a pre-defined unique/valid one
		UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMERATION),
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
		UA_QUALIFIEDNAME (0, enumName),
		ddaatt,
		NULL,
		&customEnumTypeNodeId);
	assert(st == UA_STATUSCODE_GOOD);
	// add enum values
	EnumValue enumValues[5] = {
		{
			(UA_Int64)0,
			UA_LOCALIZEDTEXT("", "Low"),
			UA_LOCALIZEDTEXT("", "Green. Low risk of attacks.")
		},
		{
			(UA_Int64)1,
			UA_LOCALIZEDTEXT("", "Guarded"),
			UA_LOCALIZEDTEXT("", "Blue. General risk of attacks.")
		},
		{
			(UA_Int64)2,
			UA_LOCALIZEDTEXT("", "Elevated"),
			UA_LOCALIZEDTEXT("", "Yellow. Significant risk of attacks.")
		},
		{
			(UA_Int64)3,
			UA_LOCALIZEDTEXT("", "High"),
			UA_LOCALIZEDTEXT("", "Orange. High risk of attacks.")
		},
		{
			(UA_Int64)4,
			UA_LOCALIZEDTEXT("", "Severe"),
			UA_LOCALIZEDTEXT("", "Red. Severe risk of attacks."),
		}
	};
	st = addEnumValues(server, &customEnumTypeNodeId, 5, enumValues);
	assert(st == UA_STATUSCODE_GOOD);
}

static void addCustomEnumVariable(UA_Server* server, char * varName) {

	UA_VariableAttributes vattr = UA_VariableAttributes_default;

	vattr.description = UA_LOCALIZEDTEXT("", varName);
	vattr.displayName = UA_LOCALIZEDTEXT("", varName);
	vattr.dataType    = customEnumTypeNodeId;
	vattr.valueRank   = -1;
	vattr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	UA_Int32 test = 1;
	UA_Variant_setScalar(&vattr.value, &test, &UA_TYPES[UA_TYPES_INT32]);

	UA_StatusCode st = UA_Server_addVariableNode(server,
		UA_NODEID_STRING (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),       
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),           
		UA_QUALIFIEDNAME (1, varName),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
		vattr,                                              
		NULL,                                               
		NULL);                                              
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

	addCustomEnumType    (server, "ThreatLevels");
	addCustomEnumVariable(server, "CurrentThreatLevel");

	UA_StatusCode retval = UA_Server_run(server, &running);
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);
	return (int)retval;
}