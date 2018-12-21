#ifndef QOPCUAABSTRACTVARIABLE_H
#define QOPCUAABSTRACTVARIABLE_H

#include <QOpcUaNodeFactory>

/*
typedef struct {                          // UA_VariableTypeAttributes_default
	// Node Attributes
	UA_UInt32        specifiedAttributes; // 0,
	UA_LocalizedText displayName;         // {{0, NULL}, {0, NULL}},
	UA_LocalizedText description;         // {{0, NULL}, {0, NULL}},
	UA_UInt32        writeMask;           // 0,
	UA_UInt32        userWriteMask;       // 0,
	// Variable Type Attributes
	UA_Variant       value;               // {NULL, UA_VARIANT_DATA, 0, NULL, 0, NULL},
	UA_NodeId        dataType;            // {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_BASEDATATYPE}},
	UA_Int32         valueRank;           // UA_VALUERANK_ANY,
	size_t           arrayDimensionsSize; // 0,
	UA_UInt32        *arrayDimensions;    // NULL,
	UA_Boolean       isAbstract;          // false
} UA_VariableTypeAttributes;

typedef struct {                              // UA_VariableAttributes_default
	// Node Attributes
	UA_UInt32        specifiedAttributes;     // 0,
	UA_LocalizedText displayName;             // {{0, NULL}, {0, NULL}},
	UA_LocalizedText description;             // {{0, NULL}, {0, NULL}},
	UA_UInt32        writeMask;               // 0,
	UA_UInt32        userWriteMask;           // 0,
	// Variable Attributes
	UA_Variant       value;                   // {NULL, UA_VARIANT_DATA, 0, NULL, 0, NULL},
	UA_NodeId        dataType;                // {0, UA_NODEIDTYPE_NUMERIC, {UA_NS0ID_BASEDATATYPE}},
	UA_Int32         valueRank;               // UA_VALUERANK_ANY,
	size_t           arrayDimensionsSize;     // 0,
	UA_UInt32        *arrayDimensions;        // NULL,
	UA_Byte          accessLevel;             // UA_ACCESSLEVELMASK_READ,
	UA_Byte          userAccessLevel;         // 0,
	UA_Double        minimumSamplingInterval; // 0.0,
	UA_Boolean       historizing;             // false
} UA_VariableAttributes;
*/

class QOpcUaAbstractVariable : public QOpcUaServerNode
{
    Q_OBJECT

	// TODO

public:
    explicit QOpcUaAbstractVariable(QOpcUaServerNode *parent);


};

#endif // QOPCUAABSTRACTVARIABLE_H