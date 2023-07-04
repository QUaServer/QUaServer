#ifndef QUABASEVARIABLE_H
#define QUABASEVARIABLE_H

#include <QUaNode>

// traits to detect if T is container and get inner_type
// NOTE : had to remove template template parameters because is c++17
template <typename T>
struct container_traits : std::false_type {};

template <typename T>
struct container_traits<QList<T>> : std::true_type
{
	using inner_type = T;
	static const QUaTypesConverter::ArrayType arrType = QUaTypesConverter::ArrayType::QList;
};

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
template <typename T>
struct container_traits<QVector<T>> : std::true_type
{
	using inner_type = T;
	static const QUaTypesConverter::ArrayType arrType = QUaTypesConverter::ArrayType::QVector;
};
#endif

template <typename T>
struct is_qvector_traits : std::false_type {};

template <typename T>
struct is_qvector_traits<QVector<T>> : std::true_type {};

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
	Q_PROPERTY(quint64 maxHistoryDataResponseSize READ maxHistoryDataResponseSize WRITE setMaxHistoryDataResponseSize)
#endif // UA_ENABLE_HISTORIZING

public:
	explicit QUaBaseVariable(
		QUaServer* server
	);

	// Attributes API

	virtual QVariant value() const;
    template<typename T>
    T value() const;

	// The data value. If the StatusCode indicates an error then the value is to be
	// ignored and the Server shall set it to null.
	// - If the new value is the same dataType or convertible to the old dataType, 
	//   the old dataType is preserved
	// - If the new value has a new type different and not convertible to the old dataType, 
	//   the dataType is updated
	// - If newDataType is defined, the new type is forced
	virtual void setValue(
		const QVariant        &value, 
		const QUaStatusCode   &statusCode      = QUaStatus::Good,
		const QDateTime       &sourceTimestamp = QDateTime(),
		const QDateTime       &serverTimestamp = QDateTime(),
		const QMetaType::Type &newDataType     = QMetaType::UnknownType
	);
	// Helper, syntactic sugar for QVariant non-supported types
	template<typename T>
	void setValue(
		const T &value, 
		const QUaStatusCode   &statusCode      = QUaStatus::Good,
		const QDateTime       &sourceTimestamp = QDateTime(),
		const QDateTime       &serverTimestamp = QDateTime(),
		const QMetaType::Type &newDataType     = QMetaType::UnknownType
	);
	// Fix for introducing helper above
	inline void setValue(
		const char            *value, 
		const QUaStatusCode   &statusCode      = QUaStatus::Good,
		const QDateTime       &sourceTimestamp = QDateTime(),
		const QDateTime       &serverTimestamp = QDateTime(),
		const QMetaType::Type &newDataType     = QMetaType::UnknownType
	)
	{
		this->setValue(
			QString::fromUtf8(value),
			statusCode,
			sourceTimestamp,
			serverTimestamp,
			newDataType
		);
	};
	// Timestamp of the source 
	virtual QDateTime sourceTimestamp() const;
	virtual void      setSourceTimestamp(const QDateTime& sourceTimestamp);
	// Timestamp when the server seceived the value from the source
	virtual QDateTime serverTimestamp() const;
	virtual void      setServerTimestamp(const QDateTime& serverTimestamp);
	// Reflects the quality of the value
	QUaStatusCode statusCode() const;
	void          setStatusCode(const QUaStatusCode& statusCode);
	// If there is no old value, a default value is assigned with the new dataType
	// If an old value exists and is convertible to the new dataType then the value is converted
	// If the old value is not convertible, then a default value is assigned with the new dataType and the old value is lost
	QMetaType::Type   dataType() const;
	void              setDataType(const QMetaType::Type &dataType);
	template<typename T>
	void              setDataTypeEnum();
	void              setDataTypeEnum(const QMetaEnum &metaEnum);
	bool              setDataTypeEnum(const QString &strEnumName);
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	bool              setDataTypeOptionSet(const QString& strOptionSetName);
#endif
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
	// Limit history data response blocks (paging). Hardcoded minimum is 50 samples.
	quint64 maxHistoryDataResponseSize() const;
	void    setMaxHistoryDataResponseSize(const quint64& maxHistoryDataResponseSize);
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
	void valueChanged          (const QVariant&      value          , const bool &networkChange);
	void statusCodeChanged     (const QUaStatusCode& statusCode     , const bool &networkChange);
	void sourceTimestampChanged(const QDateTime&     sourceTimestamp, const bool &networkChange);
	void serverTimestampChanged(const QDateTime&     serverTimestamp, const bool &networkChange);
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
#ifdef UA_ENABLE_HISTORIZING
	quint64 m_maxHistoryDataResponseSize;
#endif // UA_ENABLE_HISTORIZING

protected:

	void setDataTypeEnum(const UA_NodeId &enumTypeNodeId);
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	void setDataTypeOptionSet(const UA_NodeId &optionSetTypeNodeId);
#endif
    QMetaType::Type dataTypeInternal() const;
    // if T scalar
    template<typename T>
    T valueInternal(std::false_type) const;
    // if T array
    template<typename T>
    T valueInternal(std::true_type) const;
	// internal
	QVariant getValueInternal(
		const QUaTypesConverter::ArrayType& arrType = QUaTypesConverter::ArrayType::QList
	) const;
	UA_StatusCode setValueInternal(
		const UA_Variant    &value,
		const UA_StatusCode &status = UA_STATUSCODE_GOOD,
		const QDateTime     &sourceTimestamp = QDateTime(),
		const QDateTime     &serverTimestamp = QDateTime()
	);
};
// generic version scalar or array
template<typename T>
inline T QUaBaseVariable::value() const
{
    return this->template valueInternal<T>(is_qvector_traits<T>());
}
// if scalar
template<typename T>
inline T QUaBaseVariable::valueInternal(std::false_type) const
{
    return this->getValueInternal().template value<T>();
}
// if array
template<typename T>
inline T QUaBaseVariable::valueInternal(std::true_type) const
{
    return this->getValueInternal(QUaTypesConverter::ArrayType::QVector).template value<T>();
}

template<typename T>
inline void QUaBaseVariable::setValue(
	const T& value,
	const QUaStatusCode& statusCode,
	const QDateTime& sourceTimestamp,
	const QDateTime& serverTimestamp,
	const QMetaType::Type& newDataType)
{
	this->setValue(
		QVariant::fromValue(value),
		statusCode,
		sourceTimestamp,
		serverTimestamp,
		newDataType
	);
}

template<>
inline void QUaBaseVariable::setValue(
	const qint64& value,
	const QUaStatusCode& statusCode,
	const QDateTime& sourceTimestamp,
	const QDateTime& serverTimestamp,
	const QMetaType::Type& newDataType)
{
	this->setValue(
		QVariant::fromValue(value),
		statusCode,
		sourceTimestamp,
		serverTimestamp,
		newDataType == QMetaType::UnknownType ? QMetaType::Long : newDataType
	);
}

template<>
inline void QUaBaseVariable::setValue(
	const quint64& value,
	const QUaStatusCode& statusCode,
	const QDateTime& sourceTimestamp,
	const QDateTime& serverTimestamp,
	const QMetaType::Type& newDataType)
{
	this->setValue(
		QVariant::fromValue(value),
		statusCode,
		sourceTimestamp,
		serverTimestamp,
		newDataType == QMetaType::UnknownType ? QMetaType::ULong : newDataType
	);
}

#endif // QUABASEVARIABLE_H
