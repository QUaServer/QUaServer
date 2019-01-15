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

class QOpcUaAbstractVariable : public QOpcUaServerNode
{
    Q_OBJECT

	// Variable Attributes

	Q_PROPERTY(QVariant        value               READ get_value               WRITE set_value              )
	Q_PROPERTY(QMetaType::Type dataType            READ get_dataType            WRITE set_dataType           )
	Q_PROPERTY(qint32          valueRank           READ get_valueRank           WRITE set_valueRank          )
	Q_PROPERTY(quint32         arrayDimensionsSize READ get_arrayDimensionsSize WRITE set_arrayDimensionsSize)
	Q_PROPERTY(QList<quint32>  arrayDimensions     READ get_arrayDimensions     WRITE set_arrayDimensions    )
	Q_PROPERTY(quint8          accessLevel         READ get_accessLevel         WRITE set_accessLevel        )

	// Cannot be written from the server, as they are specific to the different users and set by the access control callback :
	// Q_PROPERTY(quint32 userAccessLevel READ get_userAccessLevel)	
	
	Q_PROPERTY(double minimumSamplingInterval READ get_minimumSamplingInterval WRITE set_minimumSamplingInterval)

	// Historizing is currently unsupported
	Q_PROPERTY(bool historizing READ get_historizing)

public:
    explicit QOpcUaAbstractVariable(QOpcUaServerNode *parent);

	QVariant        get_value                  () const;
	void            set_value                  (const QVariant &value);
	QMetaType::Type get_dataType               () const;
	void            set_dataType               (const QMetaType::Type &dataType);
	qint32          get_valueRank              () const;
	void            set_valueRank              (const qint32 &valueRank);
	quint32         get_arrayDimensionsSize    () const;
	void            set_arrayDimensionsSize    (const quint32 &arrayDimensionsSize);
	QList<quint32>  get_arrayDimensions        () const;
	void            set_arrayDimensions        (const QList<quint32> &arrayDimensions);
	quint8          get_accessLevel            () const;
	void            set_accessLevel            (const quint8 &accessLevel);
	double          get_minimumSamplingInterval() const;
	void            set_minimumSamplingInterval(const double &minimumSamplingInterval);
	bool            get_historizing            () const;

};

#endif // QOPCUAABSTRACTVARIABLE_H