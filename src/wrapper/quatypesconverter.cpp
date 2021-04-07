#include "quatypesconverter.h"
#include <cstring>

QT_BEGIN_NAMESPACE

namespace QUaTypesConverter {

UA_NodeId nodeIdFromQString(const QString & name)
{
	if (name.isEmpty())
	{
		return UA_NODEID_NULL;
	}
	quint16 namespaceIndex;
	QString identifierString;
	char    identifierType;
	bool success = nodeIdStringSplit(name.simplified().remove(' '), &namespaceIndex, &identifierString, &identifierType);

	if (!success) {
		qWarning() << "Failed to split node id string:" << name;
		return UA_NODEID_NULL;
	}

	switch (identifierType) {
	case 'i': {
		bool isNumber;
		uint identifier = identifierString.toUInt(&isNumber);
        if (isNumber)
			return UA_NODEID_NUMERIC(namespaceIndex, static_cast<UA_UInt32>(identifier));
		else
			qWarning() << name << "does not contain a valid numeric identifier";
		break;
	}
	case 's': {
		if (identifierString.length() > 0)
			return UA_NODEID_STRING_ALLOC(namespaceIndex, identifierString.toUtf8().constData());
		else
			qWarning() << name << "does not contain a valid string identifier";
		break;
	}
	case 'g': {
		QUuid uuid(identifierString);

		if (uuid.isNull()) {
			qWarning() << name << "does not contain a valid guid identifier";
			break;
		}

		UA_Guid guid;
		guid.data1 = uuid.data1;
		guid.data2 = uuid.data2;
		guid.data3 = uuid.data3;
		std::memcpy(guid.data4, uuid.data4, sizeof(uuid.data4));
		return UA_NODEID_GUID(namespaceIndex, guid);
	}
	case 'b': {
		const QByteArray temp = QByteArray::fromBase64(identifierString.toLatin1());
		if (temp.size() > 0) {
			return UA_NODEID_BYTESTRING_ALLOC(namespaceIndex, temp.constData());
		}
		else
			qWarning() << name << "does not contain a valid byte string identifier";
		break;
	}
	default:
		qWarning() << "Could not parse node id:" << name;
	}
	return UA_NODEID_NULL;
}

QString nodeIdToQString(const UA_NodeId & id)
{
	QString result = QString::fromLatin1("ns=%1;").arg(id.namespaceIndex);

    switch (id.identifierType)
    {
	case UA_NODEIDTYPE_NUMERIC:
		result.append(QString::fromLatin1("i=%1").arg(id.identifier.numeric));
		break;
	case UA_NODEIDTYPE_STRING:
		result.append(QLatin1String("s="));
		result.append(QString::fromLocal8Bit(reinterpret_cast<char *>(id.identifier.string.data), static_cast<int>(id.identifier.string.length)));
		break;
    case UA_NODEIDTYPE_GUID:
        {
		const UA_Guid &src = id.identifier.guid;
		const QUuid uuid(src.data1, src.data2, src.data3, src.data4[0], src.data4[1], src.data4[2],
			src.data4[3], src.data4[4], src.data4[5], src.data4[6], src.data4[7]);
        result.append(QStringLiteral("g=")).append(uuid.toString().midRef(1, 36));
		break;
        }
    case UA_NODEIDTYPE_BYTESTRING:
        {
		const QByteArray temp(reinterpret_cast<char *>(id.identifier.byteString.data), static_cast<int>(id.identifier.byteString.length));
		result.append(QStringLiteral("b=")).append(temp.toBase64());
		break;
        }
    }
	return result;
}

bool nodeIdStringSplit(const QString & nodeIdString, quint16 * nsIndex, QString * identifier, char * identifierType)
{
	quint16 namespaceIndex = 0;
	QStringList components = nodeIdString.split(QLatin1String(";"));
	if (components.size() > 2)
	{
		return false;
	}
	if (components.size() == 2 && components.at(0).contains(QLatin1String("ns=")))
	{
		QString strNs = components.at(0).split(QLatin1String("ns=")).last();
		bool success = false;
		uint ns = strNs.toUInt(&success);
		if (!success || ns > (std::numeric_limits<quint16>::max)())
			return false;
		namespaceIndex = ns;
	}
	auto& strLast = components.last();
	if (!strLast.contains(QLatin1String("i=")) &&
		!strLast.contains(QLatin1String("s=")) &&
		!strLast.contains(QLatin1String("g=")) &&
		!strLast.contains(QLatin1String("b=")))
	{
		return false;
	}
	auto lastParts = strLast.split(QLatin1String("="));
	if (nsIndex)
	{
		*nsIndex = namespaceIndex;
	}
	if (identifierType)
	{
		*identifierType = lastParts.first().at(0).toLatin1();
	}
	if (identifier)
	{
		*identifier = 
			lastParts.size() == 1 ?
			QString() : // NOTE : possible that just "i="
			lastParts.size() == 2 ?
			lastParts.last() : 
			lastParts.mid(1).join(QLatin1String("="));
	}
	return true;
}

QString nodeClassToQString(const UA_NodeClass & nclass)
{
	switch (nclass)
	{
	case UA_NODECLASS_UNSPECIFIED:
		return QString();
	case UA_NODECLASS_OBJECT:
		return QString("OBJECT");
	case UA_NODECLASS_VARIABLE:
		return QString("VARIABLE");
	case UA_NODECLASS_METHOD:
		return QString("METHOD");
	case UA_NODECLASS_OBJECTTYPE:
		return QString("OBJECTTYPE");
	case UA_NODECLASS_VARIABLETYPE:
		return QString("VARIABLETYPE");
	case UA_NODECLASS_REFERENCETYPE:
		return QString("REFERENCETYPE");
	case UA_NODECLASS_DATATYPE:
		return QString("DATATYPE");
	case UA_NODECLASS_VIEW:
		return QString("VIEW");
	default:
		break;
	}
	return QString();
}

QString uaStringToQString(const UA_String & string)
{
	return QString::fromLocal8Bit(reinterpret_cast<char *>(string.data), static_cast<int>(string.length));
}

UA_String uaStringFromQString(const QString & uaString)
{
	// NOTE : avoid over-use because this is expensive
	return UA_STRING_ALLOC(uaString.toUtf8().constData());
}

bool isQTypeArray(const QMetaType::Type & type)
{
	auto strTypeName = QString(QMetaType::typeName(type));
	if (strTypeName.contains("QList"  , Qt::CaseInsensitive) ||
		strTypeName.contains("QVector", Qt::CaseInsensitive))
	{
		return true;
	}
	return false;
}

QMetaType::Type getQArrayType(const QMetaType::Type & type)
{
	if (!QUaTypesConverter::isQTypeArray(type))
	{
		return QMetaType::UnknownType;
	}
	// TODO : check and use with QUaDataType::
	auto strTypeName = QString(QMetaType::typeName(type));
	strTypeName      = strTypeName.split("<").at(1);
	strTypeName      = strTypeName.split(">").at(0);
	auto byteName    = strTypeName.toUtf8();
	return (QMetaType::Type)QMetaType::type(byteName.constData());
}

bool isSupportedQType(const QMetaType::Type & type)
{
	auto typeCopy = type;
	if (QUaTypesConverter::isQTypeArray(type))
	{
		typeCopy = QUaTypesConverter::getQArrayType(type);
	}
	// NOTE : need to support QMetaType::Void for methods return value
    bool supported = typeCopy == QMetaType::Void || QUaDataType::isSupportedQType(typeCopy);
	return supported;
}

UA_NodeId uaTypeNodeIdFromQType(const QMetaType::Type & type)
{
	return QUaDataType::nodeIdByQType(type);
}

const UA_DataType * uaTypeFromQType(const QMetaType::Type & type)
{
	return QUaDataType::dataTypeByQType(type);
}

UA_Variant uaVariantFromQVariant(const QVariant & var
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
#ifndef OPEN62541_ISSUE3934_RESOLVED
	, const UA_DataType* optDataType/* = nullptr*/
#endif // !OPEN62541_ISSUE3934_RESOLVED
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
)
{
	if (!var.isValid())
	{
		UA_Variant var;
		UA_Variant_init(&var);
		return var;
	}
	// TODO : support multidimentional arrays
	QMetaType::Type qtType = QMetaType::UnknownType;
	const UA_DataType * uaType = nullptr;
	// fix qt type if array
	if (var.canConvert<QVariantList>())
	{
		qtType = QMetaType::QVariantList;
	}
	else
	{
		qtType = (QMetaType::Type)var.type();
		qtType = qtType != QMetaType::User ? qtType : (QMetaType::Type)var.userType();
		uaType = uaTypeFromQType(qtType);
	}
	// get ua type and call respective method
	switch (qtType)
	{
	case QMetaType::UnknownType:   // 23 : UA_Variant   : QVariant(QMetaType::UnknownType)
		return uaVariantFromQVariantScalar<UA_Variant   , QVariant  >(var, uaType);
	case QMetaType::Bool:          // 0  : UA_Boolean   : bool
		return uaVariantFromQVariantScalar<UA_Boolean   , bool      >(var, uaType);
	case QMetaType::Char:
	case QMetaType::SChar:         // 1  : UA_SByte     : int8_t
		return uaVariantFromQVariantScalar<UA_SByte     , char      >(var, uaType);
	case QMetaType::UChar:         // 2  : UA_Byte      : uint8_t
		return uaVariantFromQVariantScalar<UA_Byte      , uchar     >(var, uaType);
	case QMetaType::Short:         // 3  : UA_Int16     : int16_t
		return uaVariantFromQVariantScalar<UA_Int16     , qint16    >(var, uaType);
	case QMetaType::UShort:        // 4  : UA_UInt16    : uint16_t
		return uaVariantFromQVariantScalar<UA_UInt16    , quint16   >(var, uaType);
	case QMetaType::Int:           // 5  : UA_Int32     : int32_t
		return uaVariantFromQVariantScalar<UA_Int32     , qint32    >(var, uaType);
	case QMetaType::UInt:          // 6  : UA_UInt32    : uint32_t
		return uaVariantFromQVariantScalar<UA_UInt32    , quint32   >(var, uaType);
	case QMetaType::Long:
	case QMetaType::LongLong:      // 7  : UA_Int64     : int64_t
		return uaVariantFromQVariantScalar<UA_Int64     , int64_t   >(var, uaType);
	case QMetaType::ULong:
	case QMetaType::ULongLong:     // 8  : UA_UInt64    : uint64_t
		return uaVariantFromQVariantScalar<UA_UInt64    , uint64_t  >(var, uaType);
	case QMetaType::Float:         // 9  : UA_Float     : float
		return uaVariantFromQVariantScalar<UA_Float     , float     >(var, uaType);
	case QMetaType::Double:        // 10 : UA_Double    : double 
		return uaVariantFromQVariantScalar<UA_Double    , double    >(var, uaType);
	case QMetaType::QString:       // 11 : UA_String    : { size_t length;  UA_Byte *data; }
		return uaVariantFromQVariantScalar<UA_String    , QString   >(var, uaType);
	case QMetaType::QDateTime:     // 12 : UA_DateTime  : int64_t
		return uaVariantFromQVariantScalar<UA_DateTime  , QDateTime >(var, uaType);
	case QMetaType::QUuid:         // 13 : UA_Guid      : { UA_UInt32 data1; UA_UInt16 data2; UA_UInt16 data3; UA_Byte data4[8]; }
		return uaVariantFromQVariantScalar<UA_Guid      , QUuid     >(var, uaType);
	case QMetaType::QByteArray:    // 14 : UA_ByteString : UA_String * A sequence of octets. */
		return uaVariantFromQVariantScalar<UA_ByteString, QByteArray>(var, uaType);
	case QMetaType::QVariantList:   // UA_Array
		return uaVariantFromQVariantArray(var);
	default:
		{
		if(qtType == QMetaType_NodeId)           // 16 : UA_NodeId : { ... }
			return uaVariantFromQVariantScalar<UA_NodeId, QUaNodeId>(var, uaType);
		if (qtType == QMetaType_StatusCode)      // 19 : UA_StatusCode : uint32_t
			return uaVariantFromQVariantScalar<UA_StatusCode, quint32>(var, uaType);
		if (qtType == QMetaType_QualifiedName)   // 20  : UA_QualifiedName : { UA_UInt16 namespaceIndex; UA_String name; }
			return uaVariantFromQVariantScalar<UA_QualifiedName, QUaQualifiedName>(var, uaType);
		if (qtType == QMetaType_LocalizedText)   // 21 : UA_LocalizedText : { UA_String locale; UA_String text; }
			return uaVariantFromQVariantScalar<UA_LocalizedText, QUaLocalizedText>(var, uaType);
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
		if (qtType == QMetaType_Image)
			return uaVariantFromQVariantScalar<UA_ByteString, QByteArray>(var, uaType);
#ifndef OPEN62541_ISSUE3934_RESOLVED
		if (qtType == QMetaType_OptionSet)       // 108 : UA_OptionSet { UA_ByteString value; UA_ByteString validBits; }
			return uaVariantFromQVariantScalar<UA_OptionSet, QUaOptionSet>(var, optDataType);
#else
		if (qtType == QMetaType_OptionSet)       // 108 : UA_OptionSet { UA_ByteString value; UA_ByteString validBits; }
			return uaVariantFromQVariantScalar<UA_OptionSet, QUaOptionSet>(var, uaType);
#endif // !OPEN62541_ISSUE3934_RESOLVED
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		if (qtType == QMetaType_TimeZone)        // 258 : UA_TimeZoneDataType { UA_Int16 offset; UA_Boolean daylightSavingInOffset; }
			return uaVariantFromQVariantScalar<UA_TimeZoneDataType, QTimeZone>(var, uaType);
		if (qtType == QMetaType_ChangeStructureDataType) // 267 : UA_ModelChangeStructureDataType { UA_NodeId affected; UA_NodeId affectedType; UA_Byte verb; }
			return uaVariantFromQVariantScalar<UA_ModelChangeStructureDataType, QUaChangeStructureDataType>(var, uaType);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
		}
		Q_ASSERT_X(false, "uaVariantFromQVariant", "Unsupported datatype");
		return UA_Variant();
	}
}

template<typename TARGETTYPE, typename QTTYPE>
UA_Variant uaVariantFromQVariantScalar(const QVariant & var, const UA_DataType * type)
{
	UA_Variant open62541value;
	UA_Variant_init(&open62541value);
	TARGETTYPE *temp = static_cast<TARGETTYPE *>(UA_new(type));
	// T QVariant::value() const : Returns the stored value converted to the template type T. 
	// Call canConvert() to find out whether a type can be converted. 
	// If the value cannot be converted, a default-constructed value will be returned.
	uaVariantFromQVariantScalar<TARGETTYPE, QTTYPE>(var.value<QTTYPE>(), temp);
	UA_Variant_setScalar(&open62541value, temp, type);
	return open62541value;
}

template<>
UA_Variant uaVariantFromQVariantScalar<UA_Variant, QVariant>(const QVariant & var, const UA_DataType * type)
{
	Q_ASSERT(type == &UA_TYPES[UA_TYPES_VARIANT]);
	Q_UNUSED(var);
	return { NULL, UA_VARIANT_DATA, 0, NULL, 0, NULL };
}

template<typename TARGETTYPE, typename QTTYPE>
void uaVariantFromQVariantScalar(const QTTYPE & var, TARGETTYPE * ptr)
{
	*ptr = static_cast<TARGETTYPE>(var);
}
// specialization (QDateTime)
template<>
void uaVariantFromQVariantScalar<UA_DateTime, QDateTime>(const QDateTime &value, UA_DateTime *ptr)
{
	// OPC-UA part 3, Table C.9
	const QDateTime uaEpochStart(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC);
	auto time = value.toMSecsSinceEpoch();
	auto ref  = uaEpochStart.toMSecsSinceEpoch();
	*ptr = UA_DATETIME_MSEC * (time - ref); // 4.26
}
// specialization (QString)
template<>
void uaVariantFromQVariantScalar<UA_String, QString>(const QString &value, UA_String *ptr)
{
	// NOTE : avoid over-use because this is expensive
	*ptr = UA_STRING_ALLOC(value.toUtf8().constData());
}
// specialization (QByteArray)
template<>
void uaVariantFromQVariantScalar<UA_ByteString, QByteArray>(const QByteArray &value, UA_ByteString *ptr)
{
    ptr->length = static_cast<size_t>(value.length());
	UA_StatusCode success = UA_Array_copy(reinterpret_cast<const UA_Byte *>(value.constData()),
        static_cast<size_t>(value.length()), reinterpret_cast<void **>(&ptr->data), &UA_TYPES[UA_TYPES_BYTE]);
	if (success != UA_STATUSCODE_GOOD) {
		ptr->length = 0;
		ptr->data = nullptr;
	}
}
// specialization (QUuid)
template<>
void uaVariantFromQVariantScalar<UA_Guid, QUuid>(const QUuid &value, UA_Guid *ptr)
{
	ptr->data1 = value.data1;
	ptr->data2 = value.data2;
	ptr->data3 = value.data3;
	std::memcpy(ptr->data4, value.data4, sizeof(value.data4));
}
// specialization (NodeId)
template<>
void uaVariantFromQVariantScalar<UA_NodeId, QUaNodeId>(const QUaNodeId& value, UA_NodeId* ptr)
{
	*ptr = value.toUaNodeId();
}
// specialization (QualifiedName)
template<>
void uaVariantFromQVariantScalar<UA_QualifiedName, QUaQualifiedName>(const QUaQualifiedName& value, UA_QualifiedName* ptr)
{
	*ptr = value.toUaQualifiedName();
}
// specialization (LocalizedText)
template<>
void uaVariantFromQVariantScalar<UA_LocalizedText, QUaLocalizedText>(const QUaLocalizedText& value, UA_LocalizedText* ptr)
{
	*ptr = value.toUaLocalizedText();
}
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
// specialization (QUaOptionSet)
template<>
void uaVariantFromQVariantScalar<UA_OptionSet, QUaOptionSet>(const QUaOptionSet& value, UA_OptionSet* ptr)
{
	*ptr = value;
}
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
// specialization (QTimeZone)
template<>
void uaVariantFromQVariantScalar<UA_TimeZoneDataType, QTimeZone>(const QTimeZone &value, UA_TimeZoneDataType *ptr)
{
	// NOTE : this might be wrong because QTimeZone must be linked to an specific QDateTime
	ptr->offset = value.offsetFromUtc(QDateTime::currentDateTime()) / 60;
	ptr->daylightSavingInOffset = true;
}
// specialization (QUaChangeStructureDataType)
template<>
void uaVariantFromQVariantScalar<UA_ModelChangeStructureDataType, QUaChangeStructureDataType>(const QUaChangeStructureDataType &value, UA_ModelChangeStructureDataType *ptr)
{
	uaVariantFromQVariantScalar<UA_NodeId, QUaNodeId>(value.m_nodeIdAffected    , &ptr->affected    );
	uaVariantFromQVariantScalar<UA_NodeId, QUaNodeId>(value.m_nodeIdAffectedType, &ptr->affectedType);
	uaVariantFromQVariantScalar<UA_Byte  , uchar    >(value.m_uiVerb            , &ptr->verb        );
}
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

UA_Variant uaVariantFromQVariantArray(const QVariant & var)
{
	Q_ASSERT(var.canConvert<QVariantList>());
	if (!var.canConvert<QVariantList>())
	{
		return UA_Variant();
	}
	// assume that the type of the first elem of the array is the type of all the array
	auto iter = var.value<QSequentialIterable>();
	// NOTE : innerVar appears to get correct type even if var is empty array
	QVariant innerVar = iter.at(0);
	QMetaType::Type qtType = (QMetaType::Type)innerVar.type();
	qtType = qtType != QMetaType::User ? qtType : (QMetaType::Type)innerVar.userType();
	auto uaType = uaTypeFromQType(qtType);

	switch (qtType)
	{
	case QMetaType::UnknownType:   // 23 : UA_Variant   : QVariant(QMetaType::UnknownType)
		return uaVariantFromQVariantArray<UA_Variant   , QVariant  >(var, uaType);
	case QMetaType::Bool:          // 0  : UA_Boolean   : bool
		return uaVariantFromQVariantArray<UA_Boolean   , bool      >(var, uaType);
	case QMetaType::Char:
	case QMetaType::SChar:         // 1  : UA_SByte     : int8_t
		return uaVariantFromQVariantArray<UA_SByte     , char      >(var, uaType);
	case QMetaType::UChar:         // 2  : UA_Byte      : uint8_t
		return uaVariantFromQVariantArray<UA_Byte      , uchar     >(var, uaType);
	case QMetaType::Short:         // 3  : UA_Int16     : int16_t
		return uaVariantFromQVariantArray<UA_Int16     , qint16    >(var, uaType);
	case QMetaType::UShort:        // 4  : UA_UInt16    : uint16_t
		return uaVariantFromQVariantArray<UA_UInt16    , quint16   >(var, uaType);
	case QMetaType::Int:           // 5  : UA_Int32     : int32_t
		return uaVariantFromQVariantArray<UA_Int32     , qint32    >(var, uaType);
	case QMetaType::UInt:          // 6  : UA_UInt32    : uint32_t
		return uaVariantFromQVariantArray<UA_UInt32    , quint32   >(var, uaType);
	case QMetaType::Long:
	case QMetaType::LongLong:      // 7  : UA_Int64     : int64_t
		return uaVariantFromQVariantArray<UA_Int64     , int64_t   >(var, uaType);
	case QMetaType::ULong:
	case QMetaType::ULongLong:     // 8  : UA_UInt64    : uint64_t
		return uaVariantFromQVariantArray<UA_UInt64    , uint64_t  >(var, uaType);
	case QMetaType::Float:         // 9  : UA_Float     : float
		return uaVariantFromQVariantArray<UA_Float     , float     >(var, uaType);
	case QMetaType::Double:        // 10 : UA_Double    : double 
		return uaVariantFromQVariantArray<UA_Double    , double    >(var, uaType);
	case QMetaType::QString:       // 11 : UA_String    : { size_t length;  UA_Byte *data; }
		return uaVariantFromQVariantArray<UA_String    , QString   >(var, uaType);
	case QMetaType::QDateTime:     // 12 : UA_DateTime  : int64_t
		return uaVariantFromQVariantArray<UA_DateTime  , QDateTime >(var, uaType);
	case QMetaType::QUuid:         // 13 : UA_Guid      : { UA_UInt32 data1; UA_UInt16 data2; UA_UInt16 data3; UA_Byte data4[8]; }
		return uaVariantFromQVariantArray<UA_Guid      , QUuid     >(var, uaType);
	case QMetaType::QByteArray:    // 14 : UA_ByteString : UA_String * A sequence of octets. */
		return uaVariantFromQVariantArray<UA_ByteString, QByteArray>(var, uaType);
	case QMetaType::QVariantList:  // UA_Array
		Q_ASSERT_X(false, "uaVariantFromQVariantArray", "Unsupported multidimentional arrays");
		return UA_Variant();
	default:
		{
			if (qtType == QMetaType_NodeId)          // 16 : UA_NodeId : { ... }
				return uaVariantFromQVariantArray<UA_NodeId, QUaNodeId>(var, uaType);
			if (qtType == QMetaType_StatusCode)      // 19 : UA_StatusCode : uint32_t
				return uaVariantFromQVariantArray<UA_StatusCode, quint32>(var, uaType);
			if (qtType == QMetaType_QualifiedName)   // 20  : UA_QualifiedName : { UA_UInt16 namespaceIndex; UA_String name; }
				return uaVariantFromQVariantArray<UA_QualifiedName, QUaQualifiedName>(var, uaType);
			if (qtType == QMetaType_LocalizedText)   // 21 : UA_LocalizedText : { UA_String locale; UA_String text; }
				return uaVariantFromQVariantArray<UA_LocalizedText, QUaLocalizedText>(var, uaType);
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
			if (qtType == QMetaType_Image)
				return uaVariantFromQVariantArray<UA_ByteString, QByteArray>(var, uaType);
			if (qtType == QMetaType_OptionSet)        // 108 : UA_OptionSet { UA_ByteString value; UA_ByteString validBits; }
				return uaVariantFromQVariantArray<UA_OptionSet, QUaOptionSet>(var, uaType);
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
	#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
			if (qtType == QMetaType_TimeZone)        // 258 : UA_TimeZoneDataType { UA_Int16 offset; UA_Boolean daylightSavingInOffset; }
				return uaVariantFromQVariantArray<UA_TimeZoneDataType, QTimeZone>(var, uaType);
			if (qtType == QMetaType_ChangeStructureDataType) // 267 : UA_ModelChangeStructureDataType { UA_NodeId affected; UA_NodeId affectedType; UA_Byte verb; }
				return uaVariantFromQVariantArray<UA_ModelChangeStructureDataType, QUaChangeStructureDataType>(var, uaType);
	#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
		}
		Q_ASSERT_X(false, "uaVariantFromQVariant", "Unsupported datatype");
		return UA_Variant();
	}
}

template<typename TARGETTYPE, typename QTTYPE>
UA_Variant uaVariantFromQVariantArray(const QVariant & var, const UA_DataType * type)
{
	UA_Variant retVar;
	UA_Variant_init(&retVar);
	// if empty
	auto iter = var.value<QSequentialIterable>();
	if (iter.size() <= 0)
	{
		return retVar;
	}
	// instantiate ua array
    TARGETTYPE *arr = static_cast<TARGETTYPE *>(UA_Array_new(static_cast<size_t>(iter.size()), type));
	// copy values
	for (int i = 0; i < iter.size(); i++)
	{
		uaVariantFromQVariantScalar<TARGETTYPE, QTTYPE>(iter.at(i).value<QTTYPE>(), &arr[i]);
	}
	// set the array to the ua variant
	// TODO : support multidimentional array
    UA_Variant_setArray(&retVar, arr, static_cast<size_t>(iter.size()), type);
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// NOTE : UAExpert requires list of changes to be an array (not matrix)
	if (!std::is_same<QTTYPE, QUaChangeStructureDataType>::value)
	{
#endif
	// NOTE : need to disable code below if using rank and arrayDim to set size and shape of data
	//        (for UAExpert to detect if array or matrix)
		retVar.arrayDimensions     = static_cast<UA_UInt32 *>(UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]));
		retVar.arrayDimensions[0]  = static_cast<UA_UInt32>(iter.size());
		retVar.arrayDimensionsSize = static_cast<size_t>(1);
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	}
#endif
	// return ua variant
	return retVar;
}

template<>
UA_Variant uaVariantFromQVariantArray<UA_Variant, QVariant>(const QVariant & var, const UA_DataType * type)
{
	UA_Variant retVar;
	UA_Variant_init(&retVar);
	// if empty
	auto iter = var.value<QSequentialIterable>();
	if (iter.size() <= 0)
	{
		return retVar;
	}
	// instantiate ua array
	UA_Variant *arr = static_cast<UA_Variant *>(UA_Array_new(static_cast<size_t>(iter.size()), type));
	// copy values
	for (int i = 0; i < iter.size(); i++)
	{
		// NOTE : because specialization "uaVariantFromQVariantScalar<QVariant, UA_Variant>(const QVariant &value, UA_Variant *ptr)"
		//        does not work, assigning a value to UA_Variant *ptr creates a read error in client
		arr[i] = uaVariantFromQVariantScalar<UA_Variant, QVariant>(iter.at(i), &UA_TYPES[UA_TYPES_VARIANT]);
	}
	// set the array to the ua variant
	UA_Variant_setArray(&retVar, arr, static_cast<size_t>(iter.size()), type);
	// TODO : support multidimentional array
	retVar.arrayDimensions     = static_cast<UA_UInt32 *>(UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]));
	retVar.arrayDimensions[0]  = static_cast<UA_UInt32>(iter.size());
	retVar.arrayDimensionsSize = static_cast<size_t>(1);
	// return ua variant
	return retVar;
}

QMetaType::Type uaTypeNodeIdToQType(const UA_NodeId * nodeId)
{
	if (nodeId == nullptr) {
		return QMetaType::UnknownType;
	}
	return QUaDataType::qTypeByNodeId(*nodeId);
}

QMetaType::Type uaTypeToQType(const UA_DataType * uaType)
{
	if (uaType == nullptr) {
		return QMetaType::UnknownType;
	}
	return QUaDataType::qTypeByTypeIndex(uaType->typeIndex);
}

QVariant uaVariantToQVariant(const UA_Variant & uaVariant, const ArrayType& arrType /*= ArrayType::QList*/)
{
	// TODO : support multidimentional arrays

	if (uaVariant.type == nullptr) {
		return QVariant();
	}
	// first check if array
	if (!UA_Variant_isScalar(&uaVariant))
	{
		return uaVariantToQVariantArray(uaVariant, arrType);
	}
	// handle scalar
	auto index = uaVariant.type->typeIndex;
	switch (index) {
	case UA_TYPES_VARIANT:
		return uaVariantToQVariantScalar<QVariant   , UA_Variant   >(uaVariant, QMetaType::UnknownType);
	case UA_TYPES_BOOLEAN:
		return uaVariantToQVariantScalar<bool       , UA_Boolean   >(uaVariant, QMetaType::Bool);
	case UA_TYPES_SBYTE:										   
		return uaVariantToQVariantScalar<signed char, UA_SByte     >(uaVariant, QMetaType::SChar);
	case UA_TYPES_BYTE:											   
		return uaVariantToQVariantScalar<uchar      , UA_Byte      >(uaVariant, QMetaType::UChar);
	case UA_TYPES_INT16:										   
		return uaVariantToQVariantScalar<qint16     , UA_Int16     >(uaVariant, QMetaType::Short);
	case UA_TYPES_UINT16:										   
		return uaVariantToQVariantScalar<quint16    , UA_UInt16    >(uaVariant, QMetaType::UShort);
	case UA_TYPES_INT32:										   
		return uaVariantToQVariantScalar<qint32     , UA_Int32     >(uaVariant, QMetaType::Int);
	case UA_TYPES_UINT32:										   
		return uaVariantToQVariantScalar<quint32    , UA_UInt32    >(uaVariant, QMetaType::UInt);
	case UA_TYPES_INT64:										   
		return uaVariantToQVariantScalar<int64_t    , UA_Int64     >(uaVariant, QMetaType::LongLong);
	case UA_TYPES_UINT64:										   
		return uaVariantToQVariantScalar<uint64_t   , UA_UInt64    >(uaVariant, QMetaType::ULongLong);
	case UA_TYPES_FLOAT:										   
		return uaVariantToQVariantScalar<float      , UA_Float     >(uaVariant, QMetaType::Float);
	case UA_TYPES_DOUBLE:										   
		return uaVariantToQVariantScalar<double     , UA_Double    >(uaVariant, QMetaType::Double);
	case UA_TYPES_STRING:										   
		return uaVariantToQVariantScalar<QString    , UA_String    >(uaVariant, QMetaType::QString);
	case UA_TYPES_DATETIME:										   
		return uaVariantToQVariantScalar<QDateTime  , UA_DateTime  >(uaVariant, QMetaType::QDateTime);
	case UA_TYPES_GUID:											   
		return uaVariantToQVariantScalar<QUuid      , UA_Guid      >(uaVariant, QMetaType::QUuid);
	case UA_TYPES_BYTESTRING:
		return uaVariantToQVariantScalar<QByteArray , UA_ByteString>(uaVariant, QMetaType::QByteArray);
	case UA_TYPES_UTCTIME:
		// NOTE : typedef UA_DateTime UA_UtcTime;
		return uaVariantToQVariantScalar<QDateTime  , UA_DateTime  >(uaVariant, QMetaType::QDateTime);
	default:
	{
		if(index == UA_TYPES_NODEID)
			return uaVariantToQVariantScalar<QUaNodeId, UA_NodeId>(uaVariant, QMetaType_NodeId);
		if(index == UA_TYPES_STATUSCODE)
			return uaVariantToQVariantScalar<quint32, UA_StatusCode>(uaVariant, QMetaType_StatusCode);
		if(index == UA_TYPES_QUALIFIEDNAME)
			return uaVariantToQVariantScalar<QUaQualifiedName, UA_QualifiedName>(uaVariant, QMetaType_QualifiedName);
		if(index == UA_TYPES_LOCALIZEDTEXT)
			return uaVariantToQVariantScalar<QUaLocalizedText, UA_LocalizedText   >(uaVariant, QMetaType_LocalizedText);
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
		if (index == UA_TYPES_IMAGEPNG)
			return uaVariantToQVariantScalar<QByteArray, UA_ImagePNG>(uaVariant, QMetaType_Image);
		if (index == UA_TYPES_OPTIONSET)
			return uaVariantToQVariantScalar<QUaOptionSet, UA_OptionSet>(uaVariant, QMetaType_OptionSet);
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		if(index == UA_TYPES_TIMEZONEDATATYPE)
			return uaVariantToQVariantScalar<QTimeZone, UA_TimeZoneDataType>(uaVariant, QMetaType_TimeZone);
		if(index == UA_TYPES_MODELCHANGESTRUCTUREDATATYPE)
			return uaVariantToQVariantScalar<QUaChangeStructureDataType, UA_ModelChangeStructureDataType>(uaVariant, QMetaType_ChangeStructureDataType);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
			Q_ASSERT_X(false, "uaVariantToQVariant", "Unsupported datatype");
		}
	}
	return QVariant();
}

QVariant uaVariantToQVariantArray(const UA_Variant  &uaVariant, const ArrayType &arrType/* = ArrayType::QList*/)
{
	switch (arrType)
	{
	case ArrayType::QList:
		return uaVariantToQVariantList(uaVariant);
	case ArrayType::QVector:
		return uaVariantToQVariantVector(uaVariant);
	default:
        Q_ASSERT(false);
	}
    return QVariant();
}

QVariant uaVariantToQVariantList(const UA_Variant & uaVariant)
{
	Q_ASSERT(!UA_Variant_isScalar(&uaVariant));
	if (uaVariant.type == nullptr) {
		return QVariant();
	}
	// handle array
	auto index = uaVariant.type->typeIndex;
	switch (index) {
	case UA_TYPES_VARIANT:
        return uaVariantToQVariantArray<QList<QVariant>   , UA_Variant   >(uaVariant, QMetaType::UnknownType);
	case UA_TYPES_BOOLEAN:
        return uaVariantToQVariantArray<QList<bool>       , UA_Boolean   >(uaVariant, QMetaType::Bool);
	case UA_TYPES_SBYTE:										   
        return uaVariantToQVariantArray<QList<signed char>, UA_SByte     >(uaVariant, QMetaType::SChar);
	case UA_TYPES_BYTE:											   
        return uaVariantToQVariantArray<QList<uchar>      , UA_Byte      >(uaVariant, QMetaType::UChar);
	case UA_TYPES_INT16:										   
        return uaVariantToQVariantArray<QList<qint16>     , UA_Int16     >(uaVariant, QMetaType::Short);
	case UA_TYPES_UINT16:										   
        return uaVariantToQVariantArray<QList<quint16>    , UA_UInt16    >(uaVariant, QMetaType::UShort);
	case UA_TYPES_INT32:										   
        return uaVariantToQVariantArray<QList<qint32>     , UA_Int32     >(uaVariant, QMetaType::Int);
	case UA_TYPES_UINT32:										   
        return uaVariantToQVariantArray<QList<quint32>    , UA_UInt32    >(uaVariant, QMetaType::UInt);
	case UA_TYPES_INT64:										   
        return uaVariantToQVariantArray<QList<int64_t>    , UA_Int64     >(uaVariant, QMetaType::LongLong);
	case UA_TYPES_UINT64:										   
        return uaVariantToQVariantArray<QList<uint64_t>   , UA_UInt64    >(uaVariant, QMetaType::ULongLong);
	case UA_TYPES_FLOAT:										   
        return uaVariantToQVariantArray<QList<float>      , UA_Float     >(uaVariant, QMetaType::Float);
	case UA_TYPES_DOUBLE:										   
        return uaVariantToQVariantArray<QList<double>     , UA_Double    >(uaVariant, QMetaType::Double);
	case UA_TYPES_STRING:										   
        return uaVariantToQVariantArray<QList<QString>    , UA_String    >(uaVariant, QMetaType::QString);
	case UA_TYPES_DATETIME:										   
        return uaVariantToQVariantArray<QList<QDateTime>  , UA_DateTime  >(uaVariant, QMetaType::QDateTime);
	case UA_TYPES_GUID:											   
        return uaVariantToQVariantArray<QList<QUuid>      , UA_Guid      >(uaVariant, QMetaType::QUuid);
	case UA_TYPES_BYTESTRING:
        return uaVariantToQVariantArray<QList<QByteArray> , UA_ByteString>(uaVariant, QMetaType::QByteArray);
	default:
		{
		if(index == UA_TYPES_NODEID)
			return uaVariantToQVariantArray<QList<QUaNodeId>, UA_NodeId>(uaVariant, QMetaType_NodeId);
		if (index == UA_TYPES_STATUSCODE)
			return uaVariantToQVariantArray<QList<QUaStatusCode>, UA_StatusCode>(uaVariant, QMetaType_StatusCode);
		if (index == UA_TYPES_QUALIFIEDNAME)
			return uaVariantToQVariantArray<QList<QUaQualifiedName>, UA_QualifiedName>(uaVariant, QMetaType_QualifiedName);
		if (index == UA_TYPES_LOCALIZEDTEXT)
			return uaVariantToQVariantArray<QList<QUaLocalizedText>, UA_LocalizedText>(uaVariant, QMetaType_LocalizedText);
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
		if (index == UA_TYPES_IMAGEPNG)
			return uaVariantToQVariantArray<QList<QByteArray>, UA_ImagePNG>(uaVariant, QMetaType_Image);
		if (index == UA_TYPES_OPTIONSET)
			return uaVariantToQVariantArray<QList<QUaOptionSet>, UA_OptionSet>(uaVariant, QMetaType_OptionSet);
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		if (index == UA_TYPES_TIMEZONEDATATYPE)
			return uaVariantToQVariantArray<QList<QTimeZone>, UA_TimeZoneDataType>(uaVariant, QMetaType_TimeZone);
		if (index == UA_TYPES_MODELCHANGESTRUCTUREDATATYPE)
			return uaVariantToQVariantArray<QList<QUaChangeStructureDataType>, UA_ModelChangeStructureDataType>(uaVariant, QMetaType_ChangeStructureDataType);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
			Q_ASSERT_X(false, "uaVariantToQVariantArray", "Unsupported datatype");
		}
	}
	return QVariant();
}

QVariant uaVariantToQVariantVector(const UA_Variant & uaVariant)
{
	Q_ASSERT(!UA_Variant_isScalar(&uaVariant));
	if (uaVariant.type == nullptr) {
		return QVariant();
	}
	// handle array
	auto index = uaVariant.type->typeIndex;
	switch (index) {
	case UA_TYPES_VARIANT:
        return uaVariantToQVariantArray<QVector<QVariant>   , UA_Variant   >(uaVariant, QMetaType::UnknownType);
	case UA_TYPES_BOOLEAN:
        return uaVariantToQVariantArray<QVector<bool>       , UA_Boolean   >(uaVariant, QMetaType::Bool);
	case UA_TYPES_SBYTE:										   
        return uaVariantToQVariantArray<QVector<signed char>, UA_SByte     >(uaVariant, QMetaType::SChar);
	case UA_TYPES_BYTE:											   
        return uaVariantToQVariantArray<QVector<uchar>      , UA_Byte      >(uaVariant, QMetaType::UChar);
	case UA_TYPES_INT16:										   
        return uaVariantToQVariantArray<QVector<qint16>     , UA_Int16     >(uaVariant, QMetaType::Short);
	case UA_TYPES_UINT16:										   
        return uaVariantToQVariantArray<QVector<quint16>    , UA_UInt16    >(uaVariant, QMetaType::UShort);
	case UA_TYPES_INT32:										   
        return uaVariantToQVariantArray<QVector<qint32>     , UA_Int32     >(uaVariant, QMetaType::Int);
	case UA_TYPES_UINT32:										   
        return uaVariantToQVariantArray<QVector<quint32>    , UA_UInt32    >(uaVariant, QMetaType::UInt);
	case UA_TYPES_INT64:										   
        return uaVariantToQVariantArray<QVector<int64_t>    , UA_Int64     >(uaVariant, QMetaType::LongLong);
	case UA_TYPES_UINT64:										   
        return uaVariantToQVariantArray<QVector<uint64_t>   , UA_UInt64    >(uaVariant, QMetaType::ULongLong);
	case UA_TYPES_FLOAT:										   
        return uaVariantToQVariantArray<QVector<float>      , UA_Float     >(uaVariant, QMetaType::Float);
	case UA_TYPES_DOUBLE:										   
        return uaVariantToQVariantArray<QVector<double>     , UA_Double    >(uaVariant, QMetaType::Double);
	case UA_TYPES_STRING:										   
        return uaVariantToQVariantArray<QVector<QString>    , UA_String    >(uaVariant, QMetaType::QString);
	case UA_TYPES_DATETIME:										   
        return uaVariantToQVariantArray<QVector<QDateTime>  , UA_DateTime  >(uaVariant, QMetaType::QDateTime);
	case UA_TYPES_GUID:											   
        return uaVariantToQVariantArray<QVector<QUuid>      , UA_Guid      >(uaVariant, QMetaType::QUuid);
	case UA_TYPES_BYTESTRING:
        return uaVariantToQVariantArray<QVector<QByteArray> , UA_ByteString>(uaVariant, QMetaType::QByteArray);
	default:
		{
		if(index == UA_TYPES_NODEID)
			return uaVariantToQVariantArray<QVector<QUaNodeId>, UA_NodeId>(uaVariant, QMetaType_NodeId);
		if (index == UA_TYPES_STATUSCODE)
			return uaVariantToQVariantArray<QVector<QUaStatusCode>, UA_StatusCode>(uaVariant, QMetaType_StatusCode);
		if (index == UA_TYPES_QUALIFIEDNAME)
			return uaVariantToQVariantArray<QVector<QUaQualifiedName>, UA_QualifiedName>(uaVariant, QMetaType_QualifiedName);
		if (index == UA_TYPES_LOCALIZEDTEXT)
			return uaVariantToQVariantArray<QVector<QUaLocalizedText>, UA_LocalizedText>(uaVariant, QMetaType_LocalizedText);
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
		if (index == UA_TYPES_IMAGEPNG)
			return uaVariantToQVariantArray<QVector<QByteArray>, UA_ImagePNG>(uaVariant, QMetaType_Image);
		if (index == UA_TYPES_OPTIONSET)
			return uaVariantToQVariantArray<QVector<QUaOptionSet>, UA_OptionSet>(uaVariant, QMetaType_OptionSet);
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		if (index == UA_TYPES_TIMEZONEDATATYPE)
			return uaVariantToQVariantArray<QVector<QTimeZone>, UA_TimeZoneDataType>(uaVariant, QMetaType_TimeZone);
		if (index == UA_TYPES_MODELCHANGESTRUCTUREDATATYPE)
			return uaVariantToQVariantArray<QVector<QUaChangeStructureDataType>, UA_ModelChangeStructureDataType>(uaVariant, QMetaType_ChangeStructureDataType);
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
			Q_ASSERT_X(false, "uaVariantToQVariantVector", "Unsupported datatype");
		}
	}
	return QVariant();
}

template <typename ARRAYTYPE, typename UATYPE>
QVariant uaVariantToQVariantArray(const UA_Variant & var, QMetaType::Type type)
{
    using TARGETTYPE = typename ARRAYTYPE::value_type;
    ARRAYTYPE retList;
	// if empty
	if (var.arrayLength == 0 && var.data == UA_EMPTY_ARRAY_SENTINEL)
	{
		return QVariant::fromValue(retList);
	}
	// get start of array
	UATYPE *tempSrc = static_cast<UATYPE *>(var.data);
	// copy array data
	for (size_t i = 0; i < var.arrayLength; i++) 
	{
		TARGETTYPE tempTarg = uaVariantToQVariantScalar<TARGETTYPE, UATYPE>(&tempSrc[i]);
		QVariant   tempVar  = QVariant::fromValue(tempTarg);
		// convert if necessary
		if (type != QMetaType::UnknownType && 
			type != static_cast<QMetaType::Type>(tempVar.type()) &&
			type < QMetaType::User)
		{
			tempVar.convert(type);
			tempTarg = tempVar.value<TARGETTYPE>();
		}
		retList.append(tempTarg);
	}
	// return variant list
	return QVariant::fromValue(retList);
}

template<typename TARGETTYPE, typename UATYPE>
QVariant uaVariantToQVariantScalar(const UA_Variant & uaVariant, QMetaType::Type type)
{
	UATYPE  *temp    = static_cast<UATYPE *>(uaVariant.data);
	QVariant tempVar = QVariant::fromValue(uaVariantToQVariantScalar<TARGETTYPE, UATYPE>(temp));
	if (type != QMetaType::UnknownType && 
		type < QMetaType::User &&
		type != static_cast<QMetaType::Type>(tempVar.type()))
	{
		// bool QVariant::convert(int targetTypeId) : Casts the variant to the requested type, targetTypeId. 
		// If the cast cannot be done, the variant is still changed to the requested type, 
		// but is left in a cleared null state similar to that constructed by QVariant(Type).
		tempVar.convert(type);
	}
	return tempVar;
}

template<>
QVariant uaVariantToQVariantScalar<QVariant, UA_Variant>(const UA_Variant & uaVariant, QMetaType::Type type)
{
	Q_ASSERT(type == QMetaType::UnknownType);
	Q_UNUSED(uaVariant);
	return QVariant(static_cast<QVariant::Type>(QMetaType::UnknownType));
}

template<typename TARGETTYPE, typename UATYPE>
TARGETTYPE uaVariantToQVariantScalar(const UATYPE * data)
{
	return *reinterpret_cast<const TARGETTYPE *>(data);
}
// specialization (QString)
template<>
QString uaVariantToQVariantScalar<QString, UA_String>(const UA_String *data)
{
	return QString::fromUtf8(reinterpret_cast<const char *>(data->data), static_cast<int>(data->length));
}
// specialization (QByteArray)
template<>
QByteArray uaVariantToQVariantScalar<QByteArray, UA_ByteString>(const UA_ByteString *data)
{
	return QByteArray(reinterpret_cast<const char *>(data->data), static_cast<int>(data->length));
}
// specialization (QDateTime)
template<>
QDateTime uaVariantToQVariantScalar<QDateTime, UA_DateTime>(const UA_DateTime *data)
{
	// OPC-UA part 3, Table C.9
	static const QDateTime epochStart(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC);
	return epochStart.addMSecs(*data / UA_DATETIME_MSEC)/*.toLocalTime()*/;
	// TODO : why .toLocalTime() though?
}
// specialization (QUuid)
template<>
QUuid uaVariantToQVariantScalar<QUuid, UA_Guid>(const UA_Guid *data)
{
	return QUuid(data->data1, data->data2, data->data3, data->data4[0], data->data4[1], data->data4[2],
		data->data4[3], data->data4[4], data->data4[5], data->data4[6], data->data4[7]);
}
// specialization (NodeId)
template<>
QUaNodeId uaVariantToQVariantScalar<QUaNodeId, UA_NodeId>(const UA_NodeId* data)
{
	return QUaNodeId(*data);
}
// specialization (QualifiedName)
template<>
QUaQualifiedName uaVariantToQVariantScalar<QUaQualifiedName, UA_QualifiedName>(const UA_QualifiedName* data)
{
	return QUaQualifiedName(*data);
}
// specialization (LocalizedText)
template<>
QUaLocalizedText uaVariantToQVariantScalar<QUaLocalizedText, UA_LocalizedText>(const UA_LocalizedText* data)
{
	return QUaLocalizedText(*data);
}

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
// specialization (QUaOptionSet)
template<>
QUaOptionSet uaVariantToQVariantScalar<QUaOptionSet, UA_OptionSet>(const UA_OptionSet* data)
{
	return QUaOptionSet(*data);
}
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
// specialization (QTimeZone)
template<>
QTimeZone uaVariantToQVariantScalar<QTimeZone, UA_TimeZoneDataType>(const UA_TimeZoneDataType *data)
{
	// NOTE : offset is in minutes, QTimeZone receives seconds
	//        don't know what to do with the DaylightSavingInOffset flag 
	//        (true if offset includes correction, false if not)
	return QTimeZone(60 * data->offset);
}
// specialization (QUaChangeStructureDataType)
template<>
QUaChangeStructureDataType uaVariantToQVariantScalar<QUaChangeStructureDataType, UA_ModelChangeStructureDataType>(const UA_ModelChangeStructureDataType * data)
{
	QUaChangeStructureDataType ret;
	ret.m_nodeIdAffected     = uaVariantToQVariantScalar<QUaNodeId, UA_NodeId>(&data->affected    );
	ret.m_nodeIdAffectedType = uaVariantToQVariantScalar<QUaNodeId, UA_NodeId>(&data->affectedType);
	ret.m_uiVerb             = uaVariantToQVariantScalar<uchar    , UA_Byte  >(&data->verb        );
	return ret;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

template<typename T>
inline QString listToString(const QList<T>& list, const QLatin1String& separator = QLatin1String(","))
{
	QString retString;
	auto i = list.begin();
	while (i != list.end())
	{
		retString += *i;
		++i;
		if (i == list.end())
		{
			break;
		}
		retString += separator;
	}
	return retString;
}

template<typename T>
inline QList<T> stringToList(const QString& string, const QLatin1String& separator = QLatin1String(","))
{
	QList<T> retList;
	auto listChanges = string.split(separator);
	for (auto& strChange : listChanges)
	{
		retList << strChange;
	}
	return retList;
}
	
static bool registerCustomTypesRegistered = false;
void registerCustomTypes()
{
	if (registerCustomTypesRegistered)
	{
		return;
	}
	registerCustomTypesRegistered = true;
	// Qt Stuff
	Q_ASSERT(qMetaTypeId<QTimeZone>()        >= QMetaType::User);
	Q_ASSERT(qMetaTypeId<QUaReferenceType>() >= QMetaType::User);
	Q_ASSERT(qMetaTypeId<QUaEnumEntry>()     >= QMetaType::User);
	// data type
	Q_ASSERT(qMetaTypeId<QUaDataType>() >= QMetaType::User);
	QMetaType::registerConverter<QUaDataType, QString>([](QUaDataType type) {
		return type.operator QString();
	});
	QMetaType::registerConverter<QString, QUaDataType>([](QString strType) {
		return QUaDataType(strType);
	});
	// node id
	Q_ASSERT(qMetaTypeId<QUaNodeId>() >= QMetaType::User);
	QMetaType::registerConverter<QUaNodeId, QString>([](QUaNodeId nodeId) {
		return nodeId.operator QString();
	});
	QMetaType::registerConverter<QString, QUaNodeId>([](QString strNodeId) {
		return QUaNodeId(strNodeId);
	});
	// node id list
	Q_ASSERT(qMetaTypeId<QList<QUaNodeId>>() >= QMetaType::User);
	QMetaType::registerConverter<QList<QUaNodeId>, QString>([](QList<QUaNodeId> listNodeId) {
		return listToString<QUaNodeId>(listNodeId);
	});
	QMetaType::registerConverter<QString, QList<QUaNodeId>>([](QString strNodeIdList) {
		return stringToList<QUaNodeId>(strNodeIdList);
	});
	// status code
	Q_ASSERT(qMetaTypeId<QUaStatusCode>() >= QMetaType::User);
	QMetaType::registerConverter<QUaStatusCode, QString>([](QUaStatusCode statusCode) {
		return statusCode.operator QString();
	});
	QMetaType::registerConverter<QString, QUaStatusCode>([](QString strStatusCode) {
		return QUaStatusCode(strStatusCode);
	});
	// qualified name
	Q_ASSERT(qMetaTypeId<QUaQualifiedName>() >= QMetaType::User);
	QMetaType::registerConverter<QUaQualifiedName, QString>([](QUaQualifiedName qualName) {
		return qualName.operator QString();
	});
	QMetaType::registerConverter<QString, QUaQualifiedName>([](QString strQualName) {
		return QUaQualifiedName(strQualName);
	});
	// localized text
	Q_ASSERT(qMetaTypeId<QUaLocalizedText>() >= QMetaType::User);
	QMetaType::registerConverter<QUaLocalizedText, QString>([](QUaLocalizedText localText) {
		return localText.operator QString();
	});
	QMetaType::registerConverter<QString, QUaLocalizedText>([](QString strLocalText) {
		return QUaLocalizedText(strLocalText);
	});
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
	// option set
	Q_ASSERT(qMetaTypeId<QUaOptionSet>() >= QMetaType::User);
	QMetaType::registerConverter<QUaOptionSet, QString>([](QUaOptionSet optionSet) {
		return optionSet.operator QString();
	});
	QMetaType::registerConverter<QString, QUaOptionSet>([](QString strXmlOptionSet) {
		return QUaOptionSet(strXmlOptionSet);
	});
	QMetaType::registerConverter<QUaOptionSet, quint64>([](QUaOptionSet optionSet) {
		return optionSet.values();
	});
	QMetaType::registerConverter<quint64, QUaOptionSet>([](quint64 optionSetValues) {
		return QUaOptionSet(optionSetValues);
	});
	QMetaType::registerConverter<QUaOptionSet, qint64>([](QUaOptionSet optionSet) {
		return static_cast<qint32>(optionSet.values());
	});
	QMetaType::registerConverter<qint64, QUaOptionSet>([](qint64 optionSetValues) {
		return QUaOptionSet(optionSetValues);
	});
	QMetaType::registerConverter<QUaOptionSet, quint32>([](QUaOptionSet optionSet) {
		return static_cast<quint32>(optionSet.values());
	});
	QMetaType::registerConverter<quint32, QUaOptionSet>([](quint32 optionSetValues) {
		return QUaOptionSet(optionSetValues);
	});	
	QMetaType::registerConverter<QUaOptionSet, qint32>([](QUaOptionSet optionSet) {
		return static_cast<qint32>(optionSet.values());
	});
	QMetaType::registerConverter<qint32, QUaOptionSet>([](qint32 optionSetValues) {
		return QUaOptionSet(optionSetValues);
	});
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
	// node id list
	Q_ASSERT(qMetaTypeId<QList<QUaLocalizedText>>() >= QMetaType::User);
	QMetaType::registerConverter<QList<QUaLocalizedText>, QString>([](QList<QUaLocalizedText> listLocalizedText) {
		return listToString<QUaLocalizedText>(listLocalizedText, QLatin1String((char *)'\a'));
	});
	QMetaType::registerConverter<QString, QList<QUaLocalizedText>>([](QString strLocalizedTextList) {
		return stringToList<QUaLocalizedText>(strLocalizedTextList, QLatin1String((char*)'\a'));
	});
	// exclusive limit transition
	Q_ASSERT(qMetaTypeId<QUaExclusiveLimitState>() >= QMetaType::User);
	QMetaType::registerConverter<QUaExclusiveLimitState, QString>([](QUaExclusiveLimitState state) {
		return state.operator QString();
	});
	QMetaType::registerConverter<QString, QUaExclusiveLimitState>([](QString strState) {
		return QUaExclusiveLimitState(strState);
	});
	// exclusive limit transition
	Q_ASSERT(qMetaTypeId<QUaExclusiveLimitTransition>() >= QMetaType::User);
	QMetaType::registerConverter<QUaExclusiveLimitTransition, QString>([](QUaExclusiveLimitTransition transition) {
		return transition.operator QString();
	});
	QMetaType::registerConverter<QString, QUaExclusiveLimitTransition>([](QString strTransition) {
		return QUaExclusiveLimitTransition(strTransition);
	});
	// change structure
	Q_ASSERT(qMetaTypeId<QUaChangeStructureDataType>() >= QMetaType::User);
	QMetaType::registerConverter<QUaChangeStructureDataType, QString>([](QUaChangeStructureDataType change) {
		return change.operator QString();
	});
	QMetaType::registerConverter<QString, QUaChangeStructureDataType>([](QString strChange) {
		return QUaChangeStructureDataType(strChange);
	});
	// change structure list
	Q_ASSERT(qMetaTypeId<QUaChangesList>() >= QMetaType::User);
	QMetaType::registerConverter<QUaChangesList, QString>([](QUaChangesList changeList) {
		return listToString<QUaChangeStructureDataType>(changeList);
	});
	QMetaType::registerConverter<QString, QUaChangesList>([](QString strChange) {
		return stringToList<QUaChangeStructureDataType>(strChange);
	});
}

} // namespace

QT_END_NAMESPACE
