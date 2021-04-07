#include "quacustomdatatypes.h"

#include <QUaTypesConverter>

/* NOTE : for registering new custom types wrapping open62541 types follow steps below:
- Create a wrapper class for the underlying open62541 type (e.g. QUaQualifiedName for UA_QualifiedName)
- Add constructors, equality operators converting the underlying type, string for serializaton
- Provide any helper methods and member accessors, register to Qt using Q_DECLARE_METATYPE
- Add a #define in quacustomdatatypes.h for the Qt type id qMetaTypeId<T> QMetaType_QualifiedName
- Add the type's UA_NODEID_, UA_TYPES_, etc to the static hashes of QUaDataType:: (below)
- Add to quatypesconverter.h and .cpp specilzations for templated convertion methods and add to switch statements
- Register QString converters in QUaTypesConverter::registerCustomTypes using QMetaType::registerConverter

If array of types supported:
- Add a #define in quacustomdatatypes.h for the Qt type id qMetaTypeId<QList<T>>QMetaType_List_QualifiedName
- Register QString converters in QUaTypesConverter::registerCustomTypes using QMetaType::registerConverter
*/
QHash<QString, QMetaType::Type> QUaDataType::m_custTypesByName = {
	{QString("Bool")                      , QMetaType::Bool                  },
	{QString("Char")                      , QMetaType::Char                  },
	{QString("SChar")                     , QMetaType::SChar                 },
	{QString("UChar")                     , QMetaType::UChar                 },
	{QString("Short")                     , QMetaType::Short                 },
	{QString("UShort")                    , QMetaType::UShort                },
	{QString("Int")                       , QMetaType::Int                   },
	{QString("UInt")                      , QMetaType::UInt                  },
	{QString("Long")                      , QMetaType::Long                  },
	{QString("LongLong")                  , QMetaType::LongLong              },
	{QString("ULong")                     , QMetaType::ULong                 },
	{QString("ULongLong")                 , QMetaType::ULongLong             },
	{QString("Float")                     , QMetaType::Float                 },
	{QString("Double")                    , QMetaType::Double                },
	{QString("QString")                   , QMetaType::QString               },
	{QString("QDateTime")                 , QMetaType::QDateTime             },
	{QString("QUuid")                     , QMetaType::QUuid                 },
	{QString("QByteArray")                , QMetaType::QByteArray            },
	{QString("QVariant")                  , QMetaType::QVariant              },
	{QString("UnknownType")               , QMetaType::UnknownType           },
	{QString("QUaNodeId")                 , QMetaType_NodeId                 },
	{QString("QUaStatusCode")             , QMetaType_StatusCode             },
	{QString("QUaQualifiedName")          , QMetaType_QualifiedName          },
	{QString("QUaLocalizedText")          , QMetaType_LocalizedText          },
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	// TODO : image
	{QString("QImage")                    , QMetaType_Image                  },
	{QString("QUaOptionSet")              , QMetaType_OptionSet              },
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	{QString("QTimeZone")                        , QMetaType_TimeZone               },
	{QString("QUaChangeStructureDataType")       , QMetaType_ChangeStructureDataType}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
};

QHash<UA_NodeId, QMetaType::Type> QUaDataType::m_custTypesByNodeId = {
	{UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN)                     , QMetaType::Bool                  },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE)                       , QMetaType::Char                  },
	//{UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE)                       , QMetaType::SChar                 },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_BYTE)                        , QMetaType::UChar                 },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_INT16)                       , QMetaType::Short                 },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_UINT16)                      , QMetaType::UShort                },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_INT32)                       , QMetaType::Int                   },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_UINT32)                      , QMetaType::UInt                  },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_INT64)                       , QMetaType::Long                  },
	//{UA_NODEID_NUMERIC(0, UA_NS0ID_INT64)                       , QMetaType::LongLong              },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_UINT64)                      , QMetaType::ULong                 },
	//{UA_NODEID_NUMERIC(0, UA_NS0ID_UINT64)                      , QMetaType::ULongLong             },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_FLOAT)                       , QMetaType::Float                 },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE)                      , QMetaType::Double                },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_STRING)                      , QMetaType::QString               },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_DATETIME)                    , QMetaType::QDateTime             },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_UTCTIME)                     , QMetaType::QDateTime             },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_GUID)                        , QMetaType::QUuid                 },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_BYTESTRING)                  , QMetaType::QByteArray            },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE)                , QMetaType::QVariant              },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_NODEID)                      , QMetaType_NodeId                 },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_STATUSCODE)                  , QMetaType_StatusCode             },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_QUALIFIEDNAME)               , QMetaType_QualifiedName          },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_LOCALIZEDTEXT)               , QMetaType_LocalizedText          },
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	{UA_NODEID_NUMERIC(0, UA_NS0ID_IMAGE)                       , QMetaType_Image                  },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_OPTIONSET)                   , QMetaType_OptionSet              },
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	{UA_NODEID_NUMERIC(0, UA_NS0ID_TIMEZONEDATATYPE)            , QMetaType_TimeZone               },
	{UA_NODEID_NUMERIC(0, UA_NS0ID_MODELCHANGESTRUCTUREDATATYPE), QMetaType_ChangeStructureDataType}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
};

QHash<int, QMetaType::Type> QUaDataType::m_custTypesByTypeIndex = {
	{UA_TYPES_BOOLEAN                     , QMetaType::Bool                  },
	{UA_TYPES_SBYTE                       , QMetaType::Char                  },
	//{UA_TYPES_SBYTE                       , QMetaType::SChar                 },
	{UA_TYPES_BYTE                        , QMetaType::UChar                 },
	{UA_TYPES_INT16                       , QMetaType::Short                 },
	{UA_TYPES_UINT16                      , QMetaType::UShort                },
	{UA_TYPES_INT32                       , QMetaType::Int                   },
	{UA_TYPES_UINT32                      , QMetaType::UInt                  },
	{UA_TYPES_INT64                       , QMetaType::Long                  },
	//{UA_TYPES_INT64                       , QMetaType::LongLong              },
	{UA_TYPES_UINT64                      , QMetaType::ULong                 },
	//{UA_TYPES_UINT64                      , QMetaType::ULongLong             },
	{UA_TYPES_FLOAT                       , QMetaType::Float                 },
	{UA_TYPES_DOUBLE                      , QMetaType::Double                },
	{UA_TYPES_STRING                      , QMetaType::QString               },
	{UA_TYPES_DATETIME                    , QMetaType::QDateTime             },
	{UA_TYPES_GUID                        , QMetaType::QUuid                 },
	{UA_TYPES_BYTESTRING                  , QMetaType::QByteArray            },
	{UA_TYPES_VARIANT                     , QMetaType::QVariant              },
	{UA_TYPES_NODEID                      , QMetaType_NodeId                 },
	{UA_TYPES_STATUSCODE                  , QMetaType_StatusCode             },
	{UA_TYPES_QUALIFIEDNAME               , QMetaType_QualifiedName          },
	{UA_TYPES_LOCALIZEDTEXT               , QMetaType_LocalizedText          },
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	{UA_TYPES_IMAGEPNG                    , QMetaType_Image                  },
	{UA_TYPES_OPTIONSET                   , QMetaType_OptionSet              },
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	{UA_TYPES_TIMEZONEDATATYPE            , QMetaType_TimeZone               },
	{UA_TYPES_MODELCHANGESTRUCTUREDATATYPE, QMetaType_ChangeStructureDataType}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
};

QHash<QMetaType::Type, QUaDataType::TypeData> QUaDataType::m_custTypesByType = {
	{ QMetaType::Bool                   , {QString("Bool")                       , UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN)                     , &UA_TYPES[UA_TYPES_BOOLEAN                     ]} },
	{ QMetaType::Char                   , {QString("Char")                       , UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE)                       , &UA_TYPES[UA_TYPES_SBYTE                       ]} },
	{ QMetaType::SChar                  , {QString("SChar")                      , UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE)                       , &UA_TYPES[UA_TYPES_SBYTE                       ]} },
	{ QMetaType::UChar                  , {QString("UChar")                      , UA_NODEID_NUMERIC(0, UA_NS0ID_BYTE)                        , &UA_TYPES[UA_TYPES_BYTE                        ]} },
	{ QMetaType::Short                  , {QString("Short")                      , UA_NODEID_NUMERIC(0, UA_NS0ID_INT16)                       , &UA_TYPES[UA_TYPES_INT16                       ]} },
	{ QMetaType::UShort                 , {QString("UShort")                     , UA_NODEID_NUMERIC(0, UA_NS0ID_UINT16)                      , &UA_TYPES[UA_TYPES_UINT16                      ]} },
	{ QMetaType::Int                    , {QString("Int")                        , UA_NODEID_NUMERIC(0, UA_NS0ID_INT32)                       , &UA_TYPES[UA_TYPES_INT32                       ]} },
	{ QMetaType::UInt                   , {QString("UInt")                       , UA_NODEID_NUMERIC(0, UA_NS0ID_UINT32)                      , &UA_TYPES[UA_TYPES_UINT32                      ]} },
	{ QMetaType::Long                   , {QString("Long")                       , UA_NODEID_NUMERIC(0, UA_NS0ID_INT64)                       , &UA_TYPES[UA_TYPES_INT64                       ]} },
	{ QMetaType::LongLong               , {QString("LongLong")                   , UA_NODEID_NUMERIC(0, UA_NS0ID_INT64)                       , &UA_TYPES[UA_TYPES_INT64                       ]} },
	{ QMetaType::ULong                  , {QString("ULong")                      , UA_NODEID_NUMERIC(0, UA_NS0ID_UINT64)                      , &UA_TYPES[UA_TYPES_UINT64                      ]} },
	{ QMetaType::ULongLong              , {QString("ULongLong")                  , UA_NODEID_NUMERIC(0, UA_NS0ID_UINT64)                      , &UA_TYPES[UA_TYPES_UINT64                      ]} },
	{ QMetaType::Float                  , {QString("Float")                      , UA_NODEID_NUMERIC(0, UA_NS0ID_FLOAT)                       , &UA_TYPES[UA_TYPES_FLOAT                       ]} },
	{ QMetaType::Double                 , {QString("Double")                     , UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE)                      , &UA_TYPES[UA_TYPES_DOUBLE                      ]} },
	{ QMetaType::QString                , {QString("QString")                    , UA_NODEID_NUMERIC(0, UA_NS0ID_STRING)                      , &UA_TYPES[UA_TYPES_STRING                      ]} },
	{ QMetaType::QDateTime              , {QString("QDateTime")                  , UA_NODEID_NUMERIC(0, UA_NS0ID_DATETIME)                    , &UA_TYPES[UA_TYPES_DATETIME                    ]} },
	{ QMetaType::QUuid                  , {QString("QUuid")                      , UA_NODEID_NUMERIC(0, UA_NS0ID_GUID)                        , &UA_TYPES[UA_TYPES_GUID                        ]} },
	{ QMetaType::QByteArray             , {QString("QByteArray")                 , UA_NODEID_NUMERIC(0, UA_NS0ID_BYTESTRING)                  , &UA_TYPES[UA_TYPES_BYTESTRING                  ]} },
	{ QMetaType::QVariant               , {QString("QVariant")                   , UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE)                , &UA_TYPES[UA_TYPES_VARIANT                     ]} },
	{ QMetaType::UnknownType            , {QString("UnknownType")                , UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE)                , &UA_TYPES[UA_TYPES_VARIANT                     ]} },
	{ QMetaType_NodeId                  , {QString("QUaNodeId")                  , UA_NODEID_NUMERIC(0, UA_NS0ID_NODEID)                      , &UA_TYPES[UA_TYPES_NODEID                      ]} },
	{ QMetaType_StatusCode              , {QString("QUaStatusCode")              , UA_NODEID_NUMERIC(0, UA_NS0ID_STATUSCODE)                  , &UA_TYPES[UA_TYPES_STATUSCODE                  ]} },
	{ QMetaType_QualifiedName           , {QString("QUaQualifiedName")           , UA_NODEID_NUMERIC(0, UA_NS0ID_QUALIFIEDNAME)               , &UA_TYPES[UA_TYPES_QUALIFIEDNAME               ]} },
	{ QMetaType_LocalizedText           , {QString("QUaLocalizedText")           , UA_NODEID_NUMERIC(0, UA_NS0ID_LOCALIZEDTEXT)               , &UA_TYPES[UA_TYPES_LOCALIZEDTEXT               ]} },
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	// TODO : image
	{ QMetaType_Image                   , {QString("QImage")                     , UA_NODEID_NUMERIC(0, UA_NS0ID_IMAGE)                       , &UA_TYPES[UA_TYPES_IMAGEPNG                    ]} },
	{ QMetaType_OptionSet               , {QString("QUaOptionSet")              , UA_NODEID_NUMERIC(0, UA_NS0ID_OPTIONSET)                    , &UA_TYPES[UA_TYPES_OPTIONSET                   ]} },
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	{ QMetaType_TimeZone                , {QString("QTimeZone")                  , UA_NODEID_NUMERIC(0, UA_NS0ID_TIMEZONEDATATYPE)            , &UA_TYPES[UA_TYPES_TIMEZONEDATATYPE            ]} },
	{ QMetaType_ChangeStructureDataType , {QString("QUaChangeStructureDataType") , UA_NODEID_NUMERIC(0, UA_NS0ID_MODELCHANGESTRUCTUREDATATYPE), &UA_TYPES[UA_TYPES_MODELCHANGESTRUCTUREDATATYPE]} }
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
};

QUaDataType::QUaDataType()
{
	m_type = QMetaType::UnknownType;
}

QUaDataType::QUaDataType(const QMetaType::Type& metaType)
{
	m_type = metaType;
}

QUaDataType::QUaDataType(const QString& strType)
{
	*this = strType;
}

QUaDataType::operator QMetaType::Type() const
{
	return m_type;
}

QUaDataType::operator QString() const
{
	return QUaDataType::stringByQType(m_type);
}

bool QUaDataType::operator==(const QMetaType::Type& metaType)
{
	return m_type == metaType;
}

void QUaDataType::operator=(const QString& strType)
{
	Q_ASSERT_X(QUaDataType::m_custTypesByName.contains(strType), "QUaDataType", "Unknown type");
	if (!QUaDataType::m_custTypesByName.contains(strType))
	{
		m_type = QMetaType::UnknownType;
		return;
	}
	m_type = QUaDataType::m_custTypesByName[strType];
}

bool QUaDataType::isSupportedQType(const QMetaType::Type& type)
{
	return m_custTypesByType.contains(type);
}

QMetaType::Type QUaDataType::qTypeByNodeId(const UA_NodeId& nodeId)
{
#if !defined(UA_ENABLE_HISTORIZING) && !defined(UA_ENABLE_SUBSCRIPTIONS_EVENTS)
	Q_ASSERT(m_custTypesByNodeId.contains(nodeId));
#endif
	return m_custTypesByNodeId.value(nodeId, QMetaType::UnknownType);
}

QMetaType::Type QUaDataType::qTypeByTypeIndex(const int& typeIndex)
{
	Q_ASSERT(m_custTypesByTypeIndex.contains(typeIndex));
	return m_custTypesByTypeIndex.value(typeIndex, QMetaType::UnknownType);
}

UA_NodeId QUaDataType::nodeIdByQType(const QMetaType::Type& type)
{
	Q_ASSERT(m_custTypesByType.contains(type));
	if (!m_custTypesByType.contains(type))
	{
		return UA_NODEID_NULL;
	}
	return m_custTypesByType[type].nodeId;
}

const UA_DataType* QUaDataType::dataTypeByQType(const QMetaType::Type& type)
{
	Q_ASSERT(m_custTypesByType.contains(type));
	if (!m_custTypesByType.contains(type))
	{
		return nullptr;
	}
	return m_custTypesByType[type].dataType;
}

QString QUaDataType::stringByQType(const QMetaType::Type& type)
{
	Q_ASSERT(m_custTypesByType.contains(type));
	if (!m_custTypesByType.contains(type))
	{
		return QString("UnknownType");
	}
	return m_custTypesByType[type].name;
}

QMetaEnum QUaStatusCode::m_metaEnum = QMetaEnum::fromType<QUa::Status>();

QHash<QUaStatus, QString> QUaStatusCode::m_descriptions =
[]() -> QHash<QUaStatus, QString> {
	QHash<QUaStatus, QString> retHash;
	retHash[QUaStatus::Good                                   ] = QObject::tr("The operation was successful and the associated results may be used"                                       );
	retHash[QUaStatus::GoodLocalOverride                      ] = QObject::tr("The value has been overridden"                                                                             );
	retHash[QUaStatus::Uncertain                              ] = QObject::tr("The operation was partially successful and that associated results might not be suitable for some purposes");
	retHash[QUaStatus::UncertainNoCommunicationLastUsableValue] = QObject::tr("Communication to the data source has failed. The variable value is the last value that had a good quality" );
	retHash[QUaStatus::UncertainLastUsableValue               ] = QObject::tr("Whatever was updating this value has stopped doing so"                                                     );
	retHash[QUaStatus::UncertainSubstituteValue               ] = QObject::tr("The value is an operational value that was manually overwritten"                                           );
	retHash[QUaStatus::UncertainInitialValue                  ] = QObject::tr("The value is an initial value for a variable that normally receives its value from another variable"       );
	retHash[QUaStatus::UncertainSensorNotAccurate             ] = QObject::tr("The value is at one of the sensor limits"                                                                  );
	retHash[QUaStatus::UncertainEngineeringUnitsExceeded      ] = QObject::tr("The value is outside of the range of values defined for this parameter"                                    );
	retHash[QUaStatus::UncertainSubNormal                     ] = QObject::tr("The value is derived from multiple sources and has less than the required number of Good sources"            );
	retHash[QUaStatus::Bad                                    ] = QObject::tr("The operation failed and any associated results cannot be used"                                            );
	retHash[QUaStatus::BadConfigurationError                  ] = QObject::tr("There is a problem with the configuration that affects the usefulness of the value"                        );
	retHash[QUaStatus::BadNotConnected                        ] = QObject::tr("The variable should receive its value from another variable, but has never been configured to do so"       );
	retHash[QUaStatus::BadDeviceFailure                       ] = QObject::tr("There has been a failure in the device/data source that generates the value that has affected the value"   );
	retHash[QUaStatus::BadSensorFailure                       ] = QObject::tr("There has been a failure in the sensor from which the value is derived by the device/data source"          );
	retHash[QUaStatus::BadOutOfService                        ] = QObject::tr("The source of the data is not operational"                                                                 );
	retHash[QUaStatus::BadDeadbandFilterInvalid               ] = QObject::tr("The deadband filter is not valid"                                                                          );
	return retHash;
}();
QString QUaStatusCode::longDescription(const QUaStatusCode& statusCode)
{
	return QUaStatusCode::m_descriptions.value(
		statusCode, 
		QObject::tr("Unknown description value %1")
			.arg(static_cast<quint32>(statusCode))
	);
}

QUaStatusCode::QUaStatusCode()
{
	m_status = QUaStatus::Good;
}

QUaStatusCode::QUaStatusCode(const QUaStatus& uaStatus)
{
	m_status = uaStatus;
}

QUaStatusCode::QUaStatusCode(const UA_StatusCode& intStatus)
{
	m_status = static_cast<QUaStatus>(intStatus);
}

QUaStatusCode::QUaStatusCode(const QString& strStatus)
{
	*this = QUaStatusCode(strStatus.toUtf8());
}

QUaStatusCode::QUaStatusCode(const QByteArray& byteStatus)
{
	bool ok = false;
	int val = m_metaEnum.keyToValue(byteStatus.constData(), &ok);
	m_status = static_cast<QUaStatus>(val);
}

QUaStatusCode::operator QUaStatus() const
{
	return static_cast<QUaStatus>(m_status);
}

QUaStatusCode::operator UA_StatusCode() const
{
	return static_cast<UA_StatusCode>(m_status);
}

QUaStatusCode::operator QString() const
{
	const char* code = m_metaEnum.valueToKey(static_cast<int>(m_status));
	if (!code)
	{
		code = UA_StatusCode_name(static_cast<UA_StatusCode>(m_status));
	}
	return QString(code);
}

bool QUaStatusCode::operator==(const QUaStatus& uaStatus) const
{
    return m_status == uaStatus;
}

void QUaStatusCode::operator=(const QString& strStatus)
{
	*this = QUaStatusCode(strStatus.toUtf8());
}

QUaQualifiedName::QUaQualifiedName()
{
	m_namespace = 0;
	m_name = QString();
}

QUaQualifiedName::QUaQualifiedName(const quint16& namespaceIndex, const QString& name)
{
	m_namespace = namespaceIndex;
	m_name = name;
}

QUaQualifiedName::QUaQualifiedName(const UA_QualifiedName& uaQualName)
{
	// use overloaded equality operator
	*this = uaQualName;
}

QUaQualifiedName::QUaQualifiedName(const QString& strXmlQualName)
{
	// use overloaded equality operator
	*this = strXmlQualName;
}

QUaQualifiedName::QUaQualifiedName(const char* strXmlQualName)
{
	// use overloaded equality operator
	*this = strXmlQualName;
}

QUaQualifiedName::operator UA_QualifiedName() const
{
	UA_QualifiedName browseName;
	browseName.namespaceIndex = m_namespace;
	browseName.name = QUaTypesConverter::uaStringFromQString(m_name); // NOTE : allocs
	return browseName;
}

QUaQualifiedName::operator QString() const
{
	return QString("ns=%1;s=%2").arg(m_namespace).arg(m_name);
}

void QUaQualifiedName::operator=(const UA_QualifiedName& uaQualName)
{
	m_namespace = uaQualName.namespaceIndex;
	m_name = QUaTypesConverter::uaStringToQString(uaQualName.name);
}

void QUaQualifiedName::operator=(const QString& strXmlQualName)
{
	m_namespace = 0;
	auto components = QStringRef(&strXmlQualName).split(QLatin1String(";"));
	// check if valid xml format
	if (components.size() != 2)
	{
		// if no valid xml format, assume ns = 0 and given string is name
		m_name = strXmlQualName;
		return;
	}
	// check if valid namespace found, else assume ns = 0 and given string is name
    quint16 new_ns = m_namespace;
	if (components.size() == 2 && components.at(0).contains(QLatin1String("ns=")))
	{
		auto strNs = components.at(0).split(QLatin1String("ns=")).last();
		bool success = false;
		uint ns = components.at(0).mid(3).toUInt(&success);
		if (!success || ns > (std::numeric_limits<quint16>::max)())
		{
			m_name = strXmlQualName;
			return;
		}
		new_ns = ns;
	}
	// check if valid name found, else assume ns = 0 and given string is name
	auto& strLast = components.last();
	if (!strLast.contains(QLatin1String("i=")) &&
		!strLast.contains(QLatin1String("s=")) &&
		!strLast.contains(QLatin1String("g=")) &&
		!strLast.contains(QLatin1String("b=")))
	{
		m_name = strXmlQualName;
		return;
	}
	auto lastParts = strLast.split(QLatin1String("="));
	// if reached here, xml format is correct
	m_namespace = new_ns;
	m_name = 
		lastParts.size() == 1 ?
		QString() : // NOTE : possible that just "s="
		lastParts.size() == 2 ?
		lastParts.last().toString() :
		lastParts.at(1).toString();
}

void QUaQualifiedName::operator=(const char* strXmlQualName)
{
	// use overloaded equality operator
	*this = QString(strXmlQualName);
}

bool QUaQualifiedName::operator==(const QUaQualifiedName& other) const
{
	return m_namespace == other.m_namespace &&
		m_name.compare(other.m_name, Qt::CaseSensitive) == 0;
}

bool QUaQualifiedName::operator!=(const QUaQualifiedName& other) const
{
	return m_namespace != other.m_namespace ||
		m_name.compare(other.m_name, Qt::CaseSensitive) != 0;
}

bool QUaQualifiedName::operator<(const QUaQualifiedName& other) const
{
	if (m_namespace != other.m_namespace)
	{
		return m_namespace < other.m_namespace;
	}
	return m_name < other.m_name;
}

quint16 QUaQualifiedName::namespaceIndex() const
{
	return m_namespace;
}

void QUaQualifiedName::setNamespaceIndex(const quint16& index)
{
	m_namespace = index;
}

QString QUaQualifiedName::name() const
{
	return m_name;
}

void QUaQualifiedName::setName(const QString& name)
{
	m_name = name;
}

QString QUaQualifiedName::toXmlString() const
{
	// use ::operator QString()
	return *this;
}

UA_QualifiedName QUaQualifiedName::toUaQualifiedName() const
{
	// use ::operator UA_QualifiedName()
	return *this;
}

bool QUaQualifiedName::isEmpty() const
{
	return m_name.isEmpty();
}

QUaQualifiedName QUaQualifiedName::fromXmlString(const QString& strXmlQualName)
{
	return QUaQualifiedName(strXmlQualName);
}

QUaQualifiedName QUaQualifiedName::fromUaQualifiedName(const UA_QualifiedName& uaQualName)
{
	return QUaQualifiedName(uaQualName);
}

QUaBrowsePath QUaQualifiedName::saoToBrowsePath(const UA_SimpleAttributeOperand* sao)
{
	QUaBrowsePath browsePath;
	for (size_t i = 0; i < sao->browsePathSize; i++)
	{
		browsePath << sao->browsePath[i];
	}
	return browsePath;
}

QString QUaQualifiedName::reduceXml(const QUaBrowsePath& browsePath)
{
	QString strRet;
	if (browsePath.count() == 1)
	{
		return browsePath.first().toXmlString();
	}
	std::for_each(browsePath.begin(), browsePath.end(),
    [&strRet](const QUaQualifiedName& browseName) {
		strRet += "/" + browseName.toXmlString();
	});
	return strRet;
}

QString QUaQualifiedName::reduceName(
	const QUaBrowsePath& browsePath, 
	const QString& separator/* = QString("/")*/
)
{
	if (browsePath.count() == 1)
	{
		return browsePath.first().name();
	}
	QString strRet;
	auto it = browsePath.begin();
	while (it != browsePath.end())
	{
		strRet += it->name();
		if (++it != browsePath.end())
		{
			strRet += separator;
		}
	}
	return strRet;
}

QUaBrowsePath QUaQualifiedName::expandName(const QString& strPath, const QString& separator)
{
	QUaBrowsePath retPath;
	auto parts = QStringRef(&strPath).split(separator);
	for (int i = 0; i < parts.count(); i++)
	{
		retPath << QUaQualifiedName(0, parts[i].toString());
	}
	return retPath;
}

QMetaEnum QUaChangeStructureDataType::m_metaEnumVerb = QMetaEnum::fromType<QUa::ChangeVerb>();

QUaChangeStructureDataType::QUaChangeStructureDataType()
	: m_uiVerb(static_cast<uchar>(QUaChangeVerb::NodeAdded))
{
}

QUaChangeStructureDataType::QUaChangeStructureDataType(
	const QUaNodeId& nodeIdAffected,
	const QUaNodeId& nodeIdAffectedType,
	const QUaChangeVerb& uiVerb)
	: m_nodeIdAffected(nodeIdAffected),
	m_nodeIdAffectedType(nodeIdAffectedType),
	m_uiVerb(static_cast<uchar>(uiVerb))
{
}

QUaChangeStructureDataType::QUaChangeStructureDataType(const QString& strChangeStructure)
{
	auto components = QStringRef(&strChangeStructure).split(QLatin1String("|"));
	if (components.count() == 0)
	{
		return;
	}
	if (components.count() >= 1)
	{
		m_nodeIdAffected = components.at(0).toString();
	}
	if (components.count() >= 2)
	{
		m_nodeIdAffectedType = components.at(1).toString();;
	}
	if (components.count() >= 3)
	{
		bool ok = false;
		auto byte = components.at(2).toUtf8();
		int val = m_metaEnumVerb.keyToValue(byte.data(), &ok);
		m_uiVerb = ok ? static_cast<uchar>(val) : static_cast<uchar>(QUaChangeVerb::NodeAdded);
	}
}

QUaChangeStructureDataType::operator QString() const
{
	const char* verb = m_metaEnumVerb.valueToKey(static_cast<int>(m_uiVerb));
	return QString("%1|%2|%3").arg(m_nodeIdAffected).arg(m_nodeIdAffectedType).arg(verb);
}

QString QUaChangeStructureDataType::toString() const
{
	return *this;
}

QUaSession::QUaSession(QObject* parent/* = 0*/)
	: QObject(parent)
{
	m_timestamp = QDateTime::currentDateTimeUtc();
}

QString QUaSession::sessionId() const
{
	return m_strSessionId;
}

QString QUaSession::userName() const
{
	return m_strUserName;
}

QString QUaSession::applicationName() const
{
	return m_strApplicationName;
}

QString QUaSession::applicationUri() const
{
	return m_strApplicationUri;
}

QString QUaSession::productUri() const
{
	return m_strProductUri;
}

QString QUaSession::address() const
{
	return m_strAddress;
}

quint16 QUaSession::port() const
{
	return m_intPort;
}

QDateTime QUaSession::timestamp() const
{
	return m_timestamp;
}

QUaLocalizedText::QUaLocalizedText()
{
	m_locale = QString();
	m_text   = QString();
}

QUaLocalizedText::QUaLocalizedText(const QString& locale, const QString& text)
{
	m_locale = locale;
	m_text   = text;
}

QUaLocalizedText::QUaLocalizedText(const char* locale, const char* text)
{
	*this = QUaLocalizedText(QString(locale), QString(text));
}

QUaLocalizedText::QUaLocalizedText(const UA_LocalizedText& uaLocalizedText)
{
	*this = uaLocalizedText;
}

QUaLocalizedText::QUaLocalizedText(const QString& strXmlLocalizedText)
{
	*this = strXmlLocalizedText;
}

QUaLocalizedText::QUaLocalizedText(const char* strXmlLocalizedText)
{
	*this = strXmlLocalizedText;
}

QUaLocalizedText::operator UA_LocalizedText() const
{
	UA_LocalizedText uaLocalizedText;
	uaLocalizedText.locale = QUaTypesConverter::uaStringFromQString(m_locale);
	uaLocalizedText.text = QUaTypesConverter::uaStringFromQString(m_text);
	return uaLocalizedText;
}

QUaLocalizedText::operator QString() const
{
	return m_locale.isEmpty() ? m_text : QString("l=%1;t=%2").arg(m_locale).arg(m_text);
}

void QUaLocalizedText::operator=(const UA_LocalizedText& uaLocalizedText)
{
	m_locale = QUaTypesConverter::uaStringToQString(uaLocalizedText.locale);
	m_text = QUaTypesConverter::uaStringToQString(uaLocalizedText.text);
}

void QUaLocalizedText::operator=(const QString& strXmlLocalizedText)
{
	m_locale = QString();
	auto components = QStringRef(&strXmlLocalizedText).split(QLatin1String(";"));
	// check if valid xml format
	if (components.size() != 2)
	{
		// if no valid xml format, assume no-locale, and given string is text
		m_text = strXmlLocalizedText;
		return;
	}
	// check if valid locale found, else assume no-locale
	QString new_locale;
	if (components.at(0).contains(QLatin1String("l="))) 
	{
		auto partsLocale = components.at(0).split(QLatin1String("="));
		if (partsLocale.size() < 2)
		{
			m_text = strXmlLocalizedText;
			return;
		}
		new_locale = 
			partsLocale.size() == 2 ?
			partsLocale.last().toString() :
			partsLocale.at(1).toString();
	}
	// check if valid text found, else assume no-locale and given string is name
	if (!components.last().contains(QLatin1String("t=")))
	{
		m_text = strXmlLocalizedText;
		return;
	}
	auto partsText = components.last().split(QLatin1String("="));
	// if reached here, xml format is correct
	m_locale = new_locale;
	m_text =
		partsText.size() == 1 ?
		QString() : // NOTE : possible that just "t="
		partsText.size() == 2 ?
		partsText.last().toString() :
		partsText.at(1).toString();
}

void QUaLocalizedText::operator=(const char* strXmlLocalizedText)
{
	*this = QString(strXmlLocalizedText);
}

void QUaLocalizedText::operator=(const QUaLocalizedText& other)
{
	m_locale = other.m_locale;
	m_text = other.m_text;
}

bool QUaLocalizedText::operator==(const QUaLocalizedText& other) const
{
	return this->m_locale.compare(other.m_locale, Qt::CaseSensitive) == 0 &&
		this->m_text.compare(other.m_text, Qt::CaseSensitive) == 0;
}

bool QUaLocalizedText::operator<(const QUaLocalizedText& other) const
{
	if (m_locale != other.m_locale)
	{
		return m_locale < other.m_locale;
	}
	return m_text < other.m_text;
}

QString QUaLocalizedText::locale() const
{
	return m_locale;
}

void QUaLocalizedText::setLocale(const QString& locale)
{
	m_locale = locale;
}

QString QUaLocalizedText::text() const
{
	return m_text;
}

void QUaLocalizedText::setText(const QString& text)
{
	m_text = text;
}

QString QUaLocalizedText::toXmlString() const
{
	// use ::operator QString()
	return *this;
}

UA_LocalizedText QUaLocalizedText::toUaLocalizedText() const
{
	// use ::operator UA_LocalizedText()
	return *this;
}

QUaNodeId::QUaNodeId()
{
	m_nodeId = UA_NODEID_NULL;
}
QUaNodeId::QUaNodeId(const quint16& index, const quint32& numericId)
{
	m_nodeId = UA_NODEID_NULL;
	this->setNamespaceIndex(index);
	this->setNumericId(numericId);
}

QUaNodeId::QUaNodeId(const quint16& index, const QString& stringId)
{
	m_nodeId = UA_NODEID_NULL;
	this->setNamespaceIndex(index);
	this->setStringId(stringId);
}

QUaNodeId::QUaNodeId(const quint16& index, const char* stringId)
{
	m_nodeId = UA_NODEID_NULL;
	*this = QUaNodeId(index, QString(stringId));
}

QUaNodeId::QUaNodeId(const quint16& index, const QUuid& uuId)
{
	m_nodeId = UA_NODEID_NULL;
	this->setNamespaceIndex(index);
	this->setUuId(uuId);
}

QUaNodeId::QUaNodeId(const quint16& index, const QByteArray& byteArrayId)
{
	m_nodeId = UA_NODEID_NULL;
	this->setNamespaceIndex(index);
	this->setByteArrayId(byteArrayId);
}

QUaNodeId::QUaNodeId(const QUaNodeId& other)
{
	m_nodeId = UA_NODEID_NULL;
	*this = other;
}

QUaNodeId::QUaNodeId(const UA_NodeId& uaNodeId)
{
	m_nodeId = UA_NODEID_NULL;
	*this = uaNodeId;
}

QUaNodeId::QUaNodeId(const QString& strXmlNodeId)
{
	m_nodeId = UA_NODEID_NULL;
	*this = strXmlNodeId;
}

QUaNodeId::QUaNodeId(const char* strXmlNodeId)
{
	m_nodeId = UA_NODEID_NULL;
	*this = strXmlNodeId;
}

QUaNodeId::~QUaNodeId()
{
	this->clear();
}

void QUaNodeId::operator=(const UA_NodeId& uaNodeId)
{
	this->clear();
	UA_NodeId_copy(&uaNodeId, &this->m_nodeId);
}

void QUaNodeId::operator=(const QString& strXmlNodeId)
{
	this->clear();
	m_nodeId = QUaTypesConverter::nodeIdFromQString(strXmlNodeId);
}

void QUaNodeId::operator=(const char* strXmlNodeId)
{
	*this = QString(strXmlNodeId);
}

void QUaNodeId::operator=(const QUaNodeId& other)
{
	this->clear();
	UA_NodeId_copy(&other.m_nodeId, &this->m_nodeId);
}

QUaNodeId::operator UA_NodeId() const
{
	UA_NodeId retNodeId;
	UA_NodeId_copy(&m_nodeId, &retNodeId);
	return retNodeId;
}

QUaNodeId::operator QString() const
{
	return QUaTypesConverter::nodeIdToQString(m_nodeId);
}

bool QUaNodeId::operator==(const QUaNodeId& other) const
{
	return UA_NodeId_equal(&this->m_nodeId, &other.m_nodeId);
}

bool QUaNodeId::operator!=(const QUaNodeId& other) const
{
	return !UA_NodeId_equal(&this->m_nodeId, &other.m_nodeId);
}

bool QUaNodeId::operator==(const UA_NodeId& other) const
{
	return UA_NodeId_equal(&this->m_nodeId, &other);
}

bool QUaNodeId::operator<(const QUaNodeId& other) const
{
	if (m_nodeId.namespaceIndex != other.m_nodeId.namespaceIndex)
	{
		return m_nodeId.namespaceIndex < other.m_nodeId.namespaceIndex;
	}
	switch (this->type())
	{
	case QUaNodeIdType::Numeric:
		return this->numericId() < other.numericId();
	case QUaNodeIdType::String:
		return this->stringId() < other.stringId();
	case QUaNodeIdType::Guid:
		return this->uuId() < other.uuId();
	case QUaNodeIdType::ByteString:
		return this->byteArrayId() < other.byteArrayId();
	default:
		return true;
	}
}

quint16 QUaNodeId::namespaceIndex() const
{
	return m_nodeId.namespaceIndex;
}

void QUaNodeId::setNamespaceIndex(const quint16& index)
{
	m_nodeId.namespaceIndex = index;
}

QUaNodeIdType QUaNodeId::type() const
{
	return static_cast<QUaNodeIdType>(m_nodeId.identifierType);
}

quint32 QUaNodeId::numericId() const
{
	return m_nodeId.identifier.numeric;
}

void QUaNodeId::setNumericId(const quint32& numericId)
{
	if (m_nodeId.identifierType != UA_NodeIdType::UA_NODEIDTYPE_NUMERIC)
	{
		auto index = m_nodeId.namespaceIndex;
		this->clear();
		m_nodeId.namespaceIndex = index;
		m_nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_NUMERIC;
	}
	m_nodeId.identifier.numeric = numericId;
}

QString QUaNodeId::stringId() const
{
	return QUaTypesConverter::uaStringToQString(m_nodeId.identifier.string);
}

void QUaNodeId::setStringId(const QString& stringId)
{
	if (m_nodeId.identifierType != UA_NodeIdType::UA_NODEIDTYPE_STRING)
	{
		auto index = m_nodeId.namespaceIndex;
		this->clear();
		m_nodeId.namespaceIndex = index;
		m_nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_STRING;
	}
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_String, QString>(stringId, &m_nodeId.identifier.string);
}

QUuid QUaNodeId::uuId() const
{
	return QUaTypesConverter::uaVariantToQVariantScalar<QUuid, UA_Guid>(&m_nodeId.identifier.guid);
}

void QUaNodeId::setUuId(const QUuid& uuId)
{
	if (m_nodeId.identifierType != UA_NodeIdType::UA_NODEIDTYPE_GUID)
	{
		auto index = m_nodeId.namespaceIndex;
		this->clear();
		m_nodeId.namespaceIndex = index;
		m_nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_GUID;
	}
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_Guid, QUuid>(uuId, &m_nodeId.identifier.guid);
}

QByteArray QUaNodeId::byteArrayId() const
{
	return QUaTypesConverter::uaVariantToQVariantScalar<QByteArray, UA_ByteString>(&m_nodeId.identifier.byteString);
}

void QUaNodeId::setByteArrayId(const QByteArray& byteArrayId)
{
	if (m_nodeId.identifierType != UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING)
	{
		auto index = m_nodeId.namespaceIndex;
		this->clear();
		m_nodeId.namespaceIndex = index;
		m_nodeId.identifierType = UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING;
	}
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_ByteString, QByteArray>(byteArrayId, &m_nodeId.identifier.byteString);
}

QString QUaNodeId::toXmlString() const
{
	// use ::operator QString()
	return *this;
}

UA_NodeId QUaNodeId::toUaNodeId() const
{
	// use ::operator UA_NodeId()
	return *this;
}

bool QUaNodeId::isNull() const
{
	return UA_NodeId_isNull(&m_nodeId);
}

void QUaNodeId::clear()
{
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return;
	}
	UA_NodeId_clear(&m_nodeId);
	m_nodeId = UA_NODEID_NULL;
}

quint32 QUaNodeId::internalHash() const
{
	return UA_NodeId_hash(&m_nodeId);
}

QMetaEnum QUaExclusiveLimitState::m_metaEnum = QMetaEnum::fromType<QUa::ExclusiveLimitState>();

QUaExclusiveLimitState::QUaExclusiveLimitState()
{
	m_state = QUa::ExclusiveLimitState::None;
}

QUaExclusiveLimitState::QUaExclusiveLimitState(const QUa::ExclusiveLimitState& state)
{
	*this = state;
}

QUaExclusiveLimitState::QUaExclusiveLimitState(const QString& strState)
{
	*this = strState;
}

QUaExclusiveLimitState::QUaExclusiveLimitState(const char* strState)
{
	*this = strState;
}

void QUaExclusiveLimitState::operator=(const QUa::ExclusiveLimitState& state)
{
	m_state = state;
}

void QUaExclusiveLimitState::operator=(const QString& strState)
{
	QByteArray ba = strState.toUtf8();
	*this = ba.constData();
}

void QUaExclusiveLimitState::operator=(const char* strState)
{
	bool ok = false;
	int val = m_metaEnum.keyToValue(strState, &ok);
	m_state = ok ? static_cast<QUa::ExclusiveLimitState>(val) : QUa::ExclusiveLimitState::None;
}

bool QUaExclusiveLimitState::operator==(const QUaExclusiveLimitState& other) const
{
	return m_state == other.m_state;
}

bool QUaExclusiveLimitState::operator==(const QUa::ExclusiveLimitState& other) const
{
	return m_state == other;
}

QUaExclusiveLimitState::operator QUa::ExclusiveLimitState() const
{
	return m_state;
}

QUaExclusiveLimitState::operator QString() const
{
	const char* state = m_metaEnum.valueToKey(static_cast<int>(m_state));
	Q_ASSERT(state);
	return QString(state);
}

QString QUaExclusiveLimitState::toString() const
{
	return *this;
}

QMetaEnum QUaExclusiveLimitTransition::m_metaEnum = QMetaEnum::fromType<QUa::ExclusiveLimitTransition>();

QUaExclusiveLimitTransition::QUaExclusiveLimitTransition()
{
	m_transition = QUa::ExclusiveLimitTransition::Null;
}

QUaExclusiveLimitTransition::QUaExclusiveLimitTransition(const QUa::ExclusiveLimitTransition& transition)
{
	*this = transition;
}

QUaExclusiveLimitTransition::QUaExclusiveLimitTransition(const QString& strTransition)
{
	*this = strTransition;
}

QUaExclusiveLimitTransition::QUaExclusiveLimitTransition(const char* strTransition)
{
	*this = strTransition;
}

void QUaExclusiveLimitTransition::operator=(const QUa::ExclusiveLimitTransition& transition)
{
	m_transition = transition;
}

void QUaExclusiveLimitTransition::operator=(const QString& strTransition)
{
	QByteArray ba = strTransition.toUtf8();
	*this = ba.constData();
}

void QUaExclusiveLimitTransition::operator=(const char* strTransition)
{
	bool ok = false;
	int val = m_metaEnum.keyToValue(strTransition, &ok);
	m_transition = ok ? static_cast<QUa::ExclusiveLimitTransition>(val) : QUa::ExclusiveLimitTransition::Null;
}

bool QUaExclusiveLimitTransition::operator==(const QUaExclusiveLimitTransition& other) const
{
	return m_transition == other.m_transition;
}

bool QUaExclusiveLimitTransition::operator==(const QUa::ExclusiveLimitTransition& other) const
{
	return m_transition == other;
}

QUaExclusiveLimitTransition::operator QUa::ExclusiveLimitTransition() const
{
	return m_transition;
}

QUaExclusiveLimitTransition::operator QString() const
{
	const char* transition = m_metaEnum.valueToKey(static_cast<int>(m_transition));
	Q_ASSERT(transition);
	return QString(transition);
}

QString QUaExclusiveLimitTransition::toString() const
{
	return *this;
}


bool QUaEventHistoryQueryData::operator==(const QUaEventHistoryQueryData& other) const
{
	return 
		m_timeStartExisting    == other.m_timeStartExisting &&
		m_numEventsToRead      == other.m_numEventsToRead   &&
		m_numEventsAlreadyRead == other.m_numEventsAlreadyRead;
}

bool QUaEventHistoryQueryData::isValid() const
{
	return m_timeStartExisting.isValid();
}

QByteArray QUaEventHistoryQueryData::toByteArray(const QUaEventHistoryQueryData& inQueryData)
{
	QByteArray byteArray;
	QDataStream outStream(&byteArray, QIODevice::WriteOnly | QIODevice::Truncate);
	outStream.setVersion(QDataStream::Qt_5_6);
	outStream.setByteOrder(QDataStream::BigEndian);
	outStream << inQueryData;
	return byteArray;
};

QUaEventHistoryQueryData QUaEventHistoryQueryData::fromByteArray(const QByteArray& byteArray)
{
	QUaEventHistoryQueryData outQueryData;
	if (byteArray.isEmpty())
	{
		return outQueryData;
	}
	QDataStream inStream(byteArray);
	inStream.setVersion(QDataStream::Qt_5_6);
	inStream.setByteOrder(QDataStream::BigEndian);
	inStream >> outQueryData;
	return outQueryData;
}

QByteArray QUaEventHistoryQueryData::ContinuationToByteArray(const QUaEventHistoryContinuationPoint& inContinuation)
{
	QByteArray byteArray;
	QDataStream outStream(&byteArray, QIODevice::WriteOnly | QIODevice::Truncate);
	outStream.setVersion(QDataStream::Qt_5_6);
	outStream.setByteOrder(QDataStream::BigEndian);
	outStream << inContinuation;
	return byteArray;
}

QUaEventHistoryContinuationPoint QUaEventHistoryQueryData::ContinuationFromByteArray(const QByteArray& byteArray)
{
	QUaEventHistoryContinuationPoint outContinuation;
	if (byteArray.isEmpty())
	{
		return outContinuation;
	}
	QDataStream inStream(byteArray);
	inStream.setVersion(QDataStream::Qt_5_6);
	inStream.setByteOrder(QDataStream::BigEndian);
	inStream >> outContinuation;
	return outContinuation;
}

UA_ByteString QUaEventHistoryQueryData::ContinuationToUaByteString(const QUaEventHistoryContinuationPoint& inContinuation)
{
	UA_ByteString byteString;
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_ByteString, QByteArray>(
		QUaEventHistoryQueryData::ContinuationToByteArray(inContinuation),
		&byteString
		);
	return byteString;
}

QUaEventHistoryContinuationPoint QUaEventHistoryQueryData::ContinuationFromUaByteString(const UA_ByteString& uaByteArray)
{
	return QUaEventHistoryQueryData::ContinuationFromByteArray(
		QUaTypesConverter::uaVariantToQVariantScalar<QByteArray, UA_ByteString>(&uaByteArray)
	);
}

QMetaEnum QUaLog::m_metaEnumCategory = QMetaEnum::fromType<QUa::LogCategory>();
QMetaEnum QUaLog::m_metaEnumLevel = QMetaEnum::fromType<QUa::LogLevel>();

QUaLog::QUaLog()
{
	// default constructor required by Qt
	message = QByteArray();
	timestamp = QDateTime::currentDateTimeUtc();
}

QUaLog::QUaLog(const QString& strMessage,
	const QUaLogLevel& logLevel,
	const QUaLogCategory& logCategory)
{
	message = strMessage.toUtf8();
	level = logLevel;
	category = logCategory;
	timestamp = QDateTime::currentDateTimeUtc();
}

uint QUa::qHash(const Status &key, uint seed)
{
    Q_UNUSED(seed);
    return static_cast<uint>(key);
}

uint QUa::qHash(const LogLevel &key, uint seed)
{
    Q_UNUSED(seed);
    return static_cast<uint>(key);
}

uint QUa::qHash(const LogCategory &key, uint seed)
{
    Q_UNUSED(seed);
    return static_cast<uint>(key);
}

uint QUa::qHash(const ExclusiveLimitState &key, uint seed)
{
    Q_UNUSED(seed);
    return static_cast<uint>(key);
}

uint QUa::qHash(const ExclusiveLimitTransition &key, uint seed)
{
    Q_UNUSED(seed);
    return static_cast<uint>(key);
}

uint QUa::qHash(const ChangeVerb &key, uint seed)
{
    Q_UNUSED(seed);
    return static_cast<uint>(key);
}

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
QUaOptionSet::QUaOptionSet() :
	QUaOptionSet(0, 0)
{
	Q_ASSERT(m_value.size() == 8);
	Q_ASSERT(m_validBits.size() == 8);
}

QUaOptionSet::QUaOptionSet(const QUaOptionSet& other)
{
	Q_ASSERT(other.m_value.size() == 8);
	Q_ASSERT(other.m_validBits.size() == 8);
	// use overloaded equality operator
	*this = other;
	Q_ASSERT(m_value.size() == 8);
	Q_ASSERT(m_validBits.size() == 8);
}

QUaOptionSet::QUaOptionSet(const quint64& values, const quint64& validBits)
{
	this->setValues(values);
	this->setValidBits(validBits);
}

QUaOptionSet::QUaOptionSet(const UA_OptionSet& uaOptionSet)
{
	// use overloaded equality operator
	*this = uaOptionSet;
	Q_ASSERT(m_value.size() == 8);
	Q_ASSERT(m_validBits.size() == 8);
}

QUaOptionSet::QUaOptionSet(const QString& strXmlOptionSet)
{
	// use overloaded equality operator
	*this = strXmlOptionSet;
	Q_ASSERT(m_value.size() == 8);
	Q_ASSERT(m_validBits.size() == 8);
}

QUaOptionSet::QUaOptionSet(const char* strXmlOptionSet)
{
	// use overloaded equality operator
	*this = QString(strXmlOptionSet);
	Q_ASSERT(m_value.size() == 8);
	Q_ASSERT(m_validBits.size() == 8);
}

QUaOptionSet::operator UA_OptionSet() const
{
	UA_OptionSet uaOptionSet;
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_ByteString, QByteArray>(m_value    , &uaOptionSet.value);
	QUaTypesConverter::uaVariantFromQVariantScalar<UA_ByteString, QByteArray>(m_validBits, &uaOptionSet.validBits);
	return uaOptionSet;
}

QUaOptionSet::operator QString() const
{
	return QString("bits=%1;valid=%2").arg(this->values()).arg(this->validBits());
}

void QUaOptionSet::operator=(const UA_OptionSet& uaOptionSet)
{
	m_value     = QUaTypesConverter::uaVariantToQVariantScalar<QByteArray, UA_ByteString>(&uaOptionSet.value);
	m_validBits = QUaTypesConverter::uaVariantToQVariantScalar<QByteArray, UA_ByteString>(&uaOptionSet.validBits);
	Q_ASSERT(m_value.size() == 8);
	Q_ASSERT(m_validBits.size() == 8);
}

void QUaOptionSet::operator=(const QString& strXmlOptionSet)
{
	quint64 values;
	quint64 validBits;
	auto components = QStringRef(&strXmlOptionSet).split(QLatin1String(";"));
	// check if valid xml format
	if (components.size() != 2)
	{
		// if no valid xml format, assume is a number and the number is the values
		this->setValues   (strXmlOptionSet.toULongLong());
		this->setValidBits(0xFFFFFFFFFFFFFFFF);
		return;
	}
	auto firstComp   = components.at(0);
	auto firstParts  = firstComp.split(QLatin1String("="));
	auto firstPart   = firstParts.count() > 1 ? firstParts.at(1) : firstParts.at(0);
	auto secondComp  = components.at(1);
	auto secondParts = secondComp.split(QLatin1String("="));
	auto secondPart  = secondParts.count() > 1 ? secondParts.at(1) : secondParts.at(0);
	if (firstComp.contains(QString("valid")) && firstComp.contains(QString("bits")))
	{
		validBits = firstPart.toULongLong();
		values    = secondPart.toULongLong();
	}
	else
	{
		values    = firstPart.toULongLong();
		validBits = secondPart.toULongLong();
	}
	this->setValues(values);
	this->setValidBits(validBits);
}

void QUaOptionSet::operator=(const char* strXmlOptionSet)
{
	// use overloaded equality operator
	*this = QString(strXmlOptionSet);
}

void QUaOptionSet::operator=(const QUaOptionSet& other)
{
	m_value     = other.m_value;
	m_validBits = other.m_validBits;
}

bool QUaOptionSet::operator==(const QUaOptionSet& other) const
{
	return m_value == other.m_value && m_validBits == other.m_validBits;
}

bool QUaOptionSet::operator!=(const QUaOptionSet& other) const
{
	return m_value != other.m_value || m_validBits != other.m_validBits;
}

bool QUaOptionSet::operator<(const QUaOptionSet& other) const
{
	return this->values() < other.values();
}

quint64 QUaOptionSet::values() const
{
	quint64 values;
	Q_ASSERT(m_value.size() == 8);
	QDataStream inStream(m_value);
	inStream.setVersion(QDataStream::Qt_5_6);
	inStream.setByteOrder(QDataStream::LittleEndian);
	inStream >> values;
	return values;
}

void QUaOptionSet::setValues(const quint64& values)
{
	QDataStream valueStream(&m_value, QIODevice::WriteOnly | QIODevice::Truncate);
	valueStream.setVersion(QDataStream::Qt_5_6);
	valueStream.setByteOrder(QDataStream::LittleEndian);
	valueStream << static_cast<quint64>(values);
	Q_ASSERT(m_value.size() == 8);
}

quint64 QUaOptionSet::validBits() const
{
	quint64 validBits;
	Q_ASSERT(m_validBits.size() == 8);
	QDataStream inStream(m_validBits);
	inStream.setVersion(QDataStream::Qt_5_6);
	inStream.setByteOrder(QDataStream::LittleEndian);
	inStream >> validBits;
	return validBits;
}

void QUaOptionSet::setValidBits(const quint64& validBits)
{
	QDataStream validBitsStream(&m_validBits, QIODevice::WriteOnly | QIODevice::Truncate);
	validBitsStream.setVersion(QDataStream::Qt_5_6);
	validBitsStream.setByteOrder(QDataStream::LittleEndian);
	validBitsStream << static_cast<quint64>(validBits);
	Q_ASSERT(m_validBits.size() == 8);
}

// https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
bool QUaOptionSet::bitValue(const quint8& bit)
{
	return (this->values() >> bit) & 1ULL;
}

void QUaOptionSet::setBitValue(const quint8& bit, const bool& value)
{
	Q_ASSERT_X(bit < 64, "QUaOptionSet::setBitValue", "Only 64bit optionsets are supported");
	auto values = this->values();
	value ?
		values |=   1ULL << bit :
		values &= ~(1ULL << bit);
	this->setValues(values);
}

bool QUaOptionSet::bitValidity(const quint8& bit)
{
	return (this->validBits() >> bit) & 1ULL;
}

void QUaOptionSet::setBitValidity(const quint8& bit, const bool& validity)
{
	Q_ASSERT_X(bit < 64, "QUaOptionSet::setBitValidity", "Only 64bit optionsets are supported");
	auto validBits = this->validBits();
	validity ?
		validBits |=   1ULL << bit :
		validBits &= ~(1ULL << bit);
	this->setValidBits(validBits);
}

#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
