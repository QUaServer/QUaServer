#ifndef QUABASEVARIABLE_H
#define QUABASEVARIABLE_H

#include <QUaNode>

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

// Part 5 - 7.2 : BaseVariableType
/*
The BaseVariableType is the abstract base type for all other VariableTypes. 
However, only the PropertyType and the BaseDataVariableType directly inherit from this type.
*/

namespace QUa
{
	Q_NAMESPACE

	enum class Type
	{
		Bool        = QMetaType::Bool,
		Char        = QMetaType::Char,
		SChar       = QMetaType::SChar,
		UChar       = QMetaType::UChar,
		Short       = QMetaType::Short,
		UShort      = QMetaType::UShort,
		Int         = QMetaType::Int,
		UInt        = QMetaType::UInt,
		Long        = QMetaType::Long,
		LongLong    = QMetaType::LongLong,
		ULong       = QMetaType::ULong,
		ULongLong   = QMetaType::ULongLong,
		Float       = QMetaType::Float,
		Double      = QMetaType::Double,
		QString     = QMetaType::QString,
		QDateTime   = QMetaType::QDateTime,
		QUuid       = QMetaType::QUuid,
		QByteArray  = QMetaType::QByteArray,
		UnknownType = QMetaType::UnknownType,
	};
	Q_ENUM_NS(Type)
}

class QUaDataType
{

public:
	QUaDataType();
	QUaDataType(const QMetaType::Type& metaType);
	QUaDataType(const QString& strType);
	QUaDataType(const QByteArray& byteType);
	operator QMetaType::Type();
	operator QString();
	bool operator==(const QMetaType::Type& metaType);

private:
	QUa::Type m_type;
	static QMetaEnum m_metaEnum;
};

Q_DECLARE_METATYPE(QUaDataType);

class QUaBaseVariable : public QUaNode
{
	Q_OBJECT
	// Variable Attributes

	Q_PROPERTY(QVariant          value               READ value               WRITE setValue           NOTIFY valueChanged          )
	Q_PROPERTY(QUaDataType       dataType            READ dataType            WRITE setDataType    /* NOTIFY dataTypeChanged    */  )
	Q_PROPERTY(qint32            valueRank           READ valueRank           WRITE setValueRank     NOTIFY valueRankChanged       )
	Q_PROPERTY(QVector<quint32>  arrayDimensions     READ arrayDimensions    /*NOTE : Read-only      NOTIFY arrayDimensionsChanged*/)
	Q_PROPERTY(quint8            accessLevel         READ accessLevel         WRITE setAccessLevel /* NOTIFY accessLevelChanged */  )

	// Cannot be written from the server, as they are specific to the different users and set by the access control callback :
	// It is defined by overwriting the server's config->accessControl.getUserAccessLevel (see getUserAccessLevel_default)
	// Q_PROPERTY(quint32 userAccessLevel READ get_userAccessLevel)	

	Q_PROPERTY(double minimumSamplingInterval READ minimumSamplingInterval WRITE setMinimumSamplingInterval)

	// Historizing is currently unsupported
	Q_PROPERTY(bool historizing READ historizing)

public:
	explicit QUaBaseVariable(QUaServer *server);

	// Attributes API

	// If the new value is the same dataType or convertible to the old dataType, the old dataType is preserved
	// If the new value has a new type different and not convertible to the old dataType, the dataType is updated
	// Use QVariant::fromValue or use casting to force a dataType
	QVariant          value() const;
	void              setValue(const QVariant &value, QMetaType::Type newType = QMetaType::UnknownType);
	// If there is no old value, a default value is assigned with the new dataType
	// If an old value exists and is convertible to the new dataType then the value is converted
	// If the old value is not convertible, then a default value is assigned with the new dataType and the old value is lost
	QMetaType::Type   dataType() const;
	void              setDataType(const QMetaType::Type &dataType);
	template<typename T>
	void              setDataTypeEnum();
	void              setDataTypeEnum(const QMetaEnum &metaEnum);
	bool              setDataTypeEnum(const QString &strEnumName);
	// Read-only, values set automatically when calling setValue
	// NOTE : includes arrayDimensionsSize
	qint32            valueRank() const;
	void              setValueRank(const qint32& valueRank);
	QVector<quint32>  arrayDimensions() const; 
	/*
	void              setArrayDimensions(const quint32 &size); // const QVector<quint32> &arrayDimenstions
	*/
	// Indicates how the Value of a Variable can be accessed (read/write) and if it contains current and/or historic data.
	quint8            accessLevel() const;
	void              setAccessLevel(const quint8 &accessLevel);
	// The MinimumSamplingInterval Attribute indicates how “current” the Value of the Variable will be kept. 
	// It specifies (in milliseconds) how fast the Server can reasonably sample the value for changes
	double            minimumSamplingInterval() const;
	void              setMinimumSamplingInterval(const double &minimumSamplingInterval);
	// Whether the Server is actively collecting data for the history of the Variable
	// Currently unsupported by library (false)
	bool              historizing() const;
	// set callback which is called before a read is performed
	// call with the default argument for no pre-read callback
	void              setReadCallback(const std::function<QVariant()>& readCallback=std::function<QVariant()>());

	// Helpers

	// Default : read access true
	bool              readAccess() const;
	void              setReadAccess(const bool &readAccess);
	// Default : write access false
	bool              writeAccess() const;
	void              setWriteAccess(const bool &writeAccess);

	QString           dataTypeNodeId() const;

	// Static Helpers

	static qint32            GetValueRankFromQVariant      (const QVariant &varValue);
	static QVector<quint32>  GetArrayDimensionsFromQVariant(const QVariant &varValue);
	
signals:
	void valueChanged(const QVariant &value);
	void valueRankChanged(const quint32 &valueRank);

private:
	static void onWrite(UA_Server             *server, 
		                const UA_NodeId       *sessionId,
		                void                  *sessionContext, 
		                const UA_NodeId       *nodeId,
		                void                  *nodeContext, 
		                const UA_NumericRange *range,
		                const UA_DataValue    *data);

	static void onRead (UA_Server             *server, 
		                const UA_NodeId       *sessionId,
		                void                  *sessionContext, 
		                const UA_NodeId       *nodeId,
		                void                  *nodeContext, 
		                const UA_NumericRange *range,
		                const UA_DataValue    *data);

	// cache type for performance
	QMetaType::Type m_type; 
	bool m_bInternalWrite;
	std::function<QVariant()> m_readCallback;
	bool m_readCallbackRunning = false;

	void setDataTypeEnum(const UA_NodeId &enumTypeNodeId);
	QMetaType::Type dataTypeInternal() const;
};

#endif // QUABASEVARIABLE_H
