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

class QUaDataType
{

public:
    QUaDataType();
	QUaDataType(const QMetaType::Type& metaType);
    QUaDataType(const QString& strType);
    QUaDataType(const QByteArray& byteType);
	operator QMetaType::Type() const;
	operator QString() const;
	bool operator==(const QMetaType::Type& metaType);
	void operator=(const QString& strType);

private:
	QUa::Type m_type;
	static QMetaEnum m_metaEnum;
};

Q_DECLARE_METATYPE(QUaDataType);

inline uint qHash(const QUaStatus& key, uint seed)
{
	Q_UNUSED(seed);
	return static_cast<uint>(key);
}

class QUaStatusCode
{

public:
	QUaStatusCode();
	QUaStatusCode(const QUaStatus& uaStatus);
	QUaStatusCode(const UA_StatusCode& intStatus);
	QUaStatusCode(const QString& strStatus);
	QUaStatusCode(const QByteArray& byteType);
	operator QUaStatus() const;
	operator UA_StatusCode() const;
	operator QString() const;
	bool operator==(const QUaStatus& uaStatus);
	void operator=(const QString& strStatus);

	static QString longDescription(const QUaStatusCode& statusCode);

private:
	QUaStatus m_status;
	static QMetaEnum m_metaEnum;
	static QHash<QUaStatus, QString> m_descriptions;
};

Q_DECLARE_METATYPE(QUaStatusCode);

class QUaBaseVariable : public QUaNode
{
	Q_OBJECT
	// Variable Attributes

	Q_PROPERTY(QVariant          value               READ value               WRITE setValue           NOTIFY valueChanged          )
	Q_PROPERTY(QUaStatusCode     statusCode          READ statusCode          WRITE setStatusCode      NOTIFY statusCodeChanged     )
	Q_PROPERTY(QDateTime         sourceTimestamp     READ sourceTimestamp     WRITE setSourceTimestamp NOTIFY sourceTimestampChanged)
	Q_PROPERTY(QDateTime         serverTimestamp     READ serverTimestamp     WRITE setServerTimestamp NOTIFY serverTimestampChanged)
	Q_PROPERTY(QUaDataType       dataType            READ dataType            WRITE setDataType    /* NOTIFY dataTypeChanged    */  )
	Q_PROPERTY(qint32            valueRank           READ valueRank           WRITE setValueRank       NOTIFY valueRankChanged      )
	Q_PROPERTY(QVector<quint32>  arrayDimensions     READ arrayDimensions    /*NOTE : Read-only      NOTIFY arrayDimensionsChanged*/)
	Q_PROPERTY(quint8            accessLevel         READ accessLevel         WRITE setAccessLevel /* NOTIFY accessLevelChanged */  )

	// Cannot be written from the server, as they are specific to the different users and set by the access control callback :
	// It is defined by overwriting the server's config->accessControl.getUserAccessLevel (see getUserAccessLevel_default)
	// Q_PROPERTY(quint32 userAccessLevel READ get_userAccessLevel)	

	Q_PROPERTY(double minimumSamplingInterval READ minimumSamplingInterval WRITE setMinimumSamplingInterval)

#ifndef UA_ENABLE_HISTORIZING
	Q_PROPERTY(bool historizing READ historizing)
#else
	Q_PROPERTY(bool historizing READ historizing WRITE setHistorizing)
#endif // UA_ENABLE_HISTORIZING

public:
	explicit QUaBaseVariable(
		QUaServer* server
	);

	// Attributes API

	// The data value.If the StatusCode indicates an error then the value is to be
	// ignoredand the Server shall set it to null.
	// - If the new value is the same dataType or convertible to the old dataType, 
	//   the old dataType is preserved
	// - If the new value has a new type different and not convertible to the old dataType, 
	//   the dataType is updated
	// - If newDataType is defined, the new type is forced
	virtual QVariant value() const;
	virtual void     setValue(
		const QVariant        &value, 
		const QUaStatus       &statusCode      = QUaStatus::Good,
		const QDateTime       &sourceTimestamp = QDateTime(),
		const QDateTime       &serverTimestamp = QDateTime(),
		const QMetaType::Type &newDataType     = QMetaType::UnknownType
	);
	// The sourceTimestamp is used to reflect the timestamp that was applied to a Variable 
	// value by the data source. The sourceTimestamp shall be UTC time and should indicate 
	// the time of the last change of the value or statusCode.
	// In the case of a bad or uncertain status sourceTimestamp is used to reflect the time 
	// that the source recognized the non - good status or the time the Server last tried 
	// to recover from the bad or uncertain status.
	virtual QDateTime sourceTimestamp() const;
	virtual void      setSourceTimestamp(const QDateTime& sourceTimestamp);
	// The serverTimestamp is used to reflect the time that the Server received a Variable 
	// value or knew it to be accurate. In the case of a bad or uncertain status, serverTimestamp 
	// is used to reflect the time that the Server received the status or that the Server 
	// last tried to recover from the bad or uncertain status.
	virtual QDateTime serverTimestamp() const;
	virtual void      setServerTimestamp(const QDateTime& serverTimestamp);
	// The StatusCode is used to indicate the conditions under which a Variable value was generated,
	// and thereby can be used as an indicator of the usability of the value.
	// - A StatusCode with severity Good means that the value is of good quality.
	// - A StatusCode with severity Uncertain means that the quality of the value is uncertain for
	//   reasons indicated by the SubCode.
	// - A StatusCode with severity Bad means that the value is not usable for reasons indicated by
	//   the SubCode.
	// A Server, which does not support status information, shall return a severity code of Good. 
	// It is also acceptable for a Server to simply return a severity and a non - specific(0) SubCode.
	// If the Server has no known value - in particular when Severity is BAD, it shall return a
	// NULL value
	QUaStatus statusCode() const;
	void      setStatusCode(const QUaStatus& statusCode);
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
	bool              historizing() const;
#ifdef UA_ENABLE_HISTORIZING
	void              setHistorizing(const bool & historizing);
#endif // UA_ENABLE_HISTORIZING
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

#ifdef UA_ENABLE_HISTORIZING
	// Default : read history access false
	bool              readHistoryAccess() const;
	void              setReadHistoryAccess(const bool& readHistoryAccess);
	// Default : write history access false
	bool              writeHistoryAccess() const;
	void              setWriteHistoryAccess(const bool& writeHistoryAccess);
#endif // UA_ENABLE_HISTORIZING

	QString           dataTypeNodeId() const;

	// Static Helpers

	static qint32            GetValueRankFromQVariant      (const QVariant &varValue);
	static QVector<quint32>  GetArrayDimensionsFromQVariant(const QVariant &varValue);

signals:
	void valueChanged          (const QVariant&      value          );
	void statusCodeChanged     (const QUaStatusCode& statusCode     );
	void sourceTimestampChanged(const QDateTime&     sourceTimestamp);
	void serverTimestampChanged(const QDateTime&     serverTimestamp);
	void valueRankChanged      (const quint32&       valueRank      );

protected:
	// cache type for performance
	QMetaType::Type m_dataType;

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

	bool m_bInternalWrite;
	std::function<QVariant()> m_readCallback;
	bool m_readCallbackRunning = false;

	protected:

	void setDataTypeEnum(const UA_NodeId &enumTypeNodeId);
	QMetaType::Type dataTypeInternal() const;
	UA_StatusCode setValueInternal(
		const UA_Variant    &value,
		const UA_StatusCode &status = UA_STATUSCODE_GOOD,
		const QDateTime     &sourceTimestamp = QDateTime(),
		const QDateTime     &serverTimestamp = QDateTime()
	);
};

#endif // QUABASEVARIABLE_H
