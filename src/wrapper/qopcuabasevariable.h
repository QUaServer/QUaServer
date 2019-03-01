#ifndef QOPCUABASEVARIABLE_H
#define QOPCUABASEVARIABLE_H

#include <QOpcUaServerNode>

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


 * Access Level Masks
 * ------------------
 * The access level to a node is given by the following constants that are ANDed
 * with the overall access level.

#define UA_ACCESSLEVELMASK_READ           (0x01<<0)
#define UA_ACCESSLEVELMASK_WRITE          (0x01<<1)
#define UA_ACCESSLEVELMASK_HISTORYREAD    (0x01<<2)
#define UA_ACCESSLEVELMASK_HISTORYWRITE   (0x01<<3)
#define UA_ACCESSLEVELMASK_SEMANTICCHANGE (0x01<<4)
#define UA_ACCESSLEVELMASK_STATUSWRITE    (0x01<<5)
#define UA_ACCESSLEVELMASK_TIMESTAMPWRITE (0x01<<6)

*/

// Part 5 - 7.2 : BaseVariableType
/*
The BaseVariableType is the abstract base type for all other VariableTypes. 
However, only the PropertyType and the BaseDataVariableType directly inherit from this type.
*/

class QOpcUaBaseVariable : public QOpcUaServerNode
{
	Q_OBJECT

	// Variable Attributes

	Q_PROPERTY(QVariant          value               READ get_value               WRITE set_value           NOTIFY valueChanged          )
	Q_PROPERTY(QMetaType::Type   dataType            READ get_dataType            WRITE set_dataType        NOTIFY dataTypeChanged       )
	Q_PROPERTY(qint32            valueRank           READ get_valueRank          /*NOTE : Read-only*/       NOTIFY valueRankChanged      )
	Q_PROPERTY(QVector<quint32>  arrayDimensions     READ get_arrayDimensions    /*NOTE : Read-only*/       NOTIFY arrayDimensionsChanged)
	Q_PROPERTY(quint8            accessLevel         READ get_accessLevel         WRITE set_accessLevel     NOTIFY accessLevelChanged    )

	// Cannot be written from the server, as they are specific to the different users and set by the access control callback :
	// Q_PROPERTY(quint32 userAccessLevel READ get_userAccessLevel)	

	Q_PROPERTY(double minimumSamplingInterval READ get_minimumSamplingInterval WRITE set_minimumSamplingInterval)

	// Historizing is currently unsupported
	Q_PROPERTY(bool historizing READ get_historizing)

public:
	
	// If the new value is the same dataType or convertible to the old dataType, the old dataType is preserved
	// If the new value has a new type different and not convertible to the old dataType, the dataType is updated
	// Use QVariant::fromValue or use casting to force a dataType
	QVariant          get_value() const;
	void              set_value(const QVariant &value);
	// If there is no old value, a default value is assigned with the new dataType
	// If an old value exists and is convertible to the new dataType then the value is converted
	// If the old value is not convertible, then a default value is assigned with the new dataType and the old value is lost
	QMetaType::Type   get_dataType() const;
	void              set_dataType(const QMetaType::Type &dataType);
	// Read-only, values set automatically when calling set_value
	qint32            get_valueRank() const;
	QVector<quint32>  get_arrayDimensions() const; // includes arrayDimensionsSize
	// 
	quint8            get_accessLevel() const;
	void              set_accessLevel(const quint8 &accessLevel);
	// 
	double            get_minimumSamplingInterval() const;
	void              set_minimumSamplingInterval(const double &minimumSamplingInterval);
	// 
	bool              get_historizing() const;

	// static helpers

	static qint32            GetValueRankFromQVariant      (const QVariant &varValue);
	static QVector<quint32>  GetArrayDimensionsFromQVariant(const QVariant &varValue);
	
signals:
	void valueChanged          (const QVariant         &value          );
	void dataTypeChanged       (const QMetaType::Type  &dataType       );
	void valueRankChanged      (const qint32           &valueRank      );
	void arrayDimensionsChanged(const QVector<quint32> &arrayDimensions);
	void accessLevelChanged    (const quint8           &accessLevel    );

};


#endif // QOPCUABASEVARIABLE_H