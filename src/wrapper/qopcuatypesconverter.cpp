#include "qopcuatypesconverter.h"
#include <cstring>

QT_BEGIN_NAMESPACE

namespace QOpcUaTypesConverter {


	UA_NodeId nodeIdFromQString(const QString & name)
	{
		quint16 namespaceIndex;
		QString identifierString;
		char    identifierType;
		bool success = nodeIdStringSplit(name, &namespaceIndex, &identifierString, &identifierType);

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
			return false;

		if (components.size() == 2 && components.at(0).contains(QRegularExpression(QLatin1String("^ns=[0-9]+")))) {
			bool success = false;
			uint ns = components.at(0).midRef(3).toString().toUInt(&success);
			if (!success || ns > (std::numeric_limits<quint16>::max)())
				return false;
			namespaceIndex = ns;
		}

		if (components.last().size() < 3)
			return false;

		if (!components.last().contains(QRegularExpression(QLatin1String("^[isgb]="))))
			return false;

		if (nsIndex)
			*nsIndex = namespaceIndex;
		if (identifier)
			*identifier = components.last().midRef(2).toString();
		if (identifierType)
			*identifierType = components.last().at(0).toLatin1();

		return true;
	}

	QString nodeClassToQString(const UA_NodeClass & nclass)
	{
		switch (nclass)
		{
		case UA_NODECLASS_UNSPECIFIED:
			return QString();
			break;
		case UA_NODECLASS_OBJECT:
			return QString("OBJECT");
			break;
		case UA_NODECLASS_VARIABLE:
			return QString("VARIABLE");
			break;
		case UA_NODECLASS_METHOD:
			return QString("METHOD");
			break;
		case UA_NODECLASS_OBJECTTYPE:
			return QString("OBJECTTYPE");
			break;
		case UA_NODECLASS_VARIABLETYPE:
			return QString("VARIABLETYPE");
			break;
		case UA_NODECLASS_REFERENCETYPE:
			return QString("REFERENCETYPE");
			break;
		case UA_NODECLASS_DATATYPE:
			return QString("DATATYPE");
			break;
		case UA_NODECLASS_VIEW:
			return QString("VIEW");
			break;
		default:
			return QString();
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
		return UA_STRING_ALLOC(uaString.toUtf8().constData());
	}

	bool isSupportedQType(const QMetaType::Type & type)
	{
		switch (type)
		{
		case QMetaType::UnknownType:
		case QMetaType::Bool:
		case QMetaType::Char:
		case QMetaType::SChar:
		case QMetaType::UChar:
		case QMetaType::Short:
		case QMetaType::UShort:
		case QMetaType::Int:
		case QMetaType::UInt:
		case QMetaType::Long:
		case QMetaType::LongLong:
		case QMetaType::ULong:
		case QMetaType::ULongLong:
		case QMetaType::Float:
		case QMetaType::Double:
		case QMetaType::QString:
		case QMetaType::QDateTime:
		case QMetaType::QUuid:
		case QMetaType::QByteArray:
			return true;
		// TODO : QMetaType::QVariantList ???
		default:
			Q_ASSERT_X(false, "uaTypeNodeIdFromQType", "Unsupported datatype");
			return false;
		}
	}

	UA_NodeId uaTypeNodeIdFromQType(const QMetaType::Type & type)
	{
		switch (type)
		{
		case QMetaType::UnknownType:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE ); // 23 : UA_Variant : QVariant(QMetaType::UnknownType)
		case QMetaType::Bool:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN   );    // 0  : UA_Boolean   : bool
		case QMetaType::Char:
		case QMetaType::SChar:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE     );    // 1  : UA_SByte     : int8_t
		case QMetaType::UChar:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BYTE      );    // 2  : UA_Byte      : uint8_t
		case QMetaType::Short:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_INT16     );    // 3  : UA_Int16     : int16_t
		case QMetaType::UShort:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_UINT16    );    // 4  : UA_UInt16    : uint16_t
		case QMetaType::Int:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_INT32     );    // 5  : UA_Int32     : int32_t
		case QMetaType::UInt:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_UINT32    );    // 6  : UA_UInt32    : uint32_t
		case QMetaType::Long:
		case QMetaType::LongLong:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_INT64     );    // 7  : UA_Int64     : int64_t
		case QMetaType::ULong:
		case QMetaType::ULongLong:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_UINT64    );    // 8  : UA_UInt64    : uint64_t
		case QMetaType::Float:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_FLOAT     );    // 9  : UA_Float     : float
		case QMetaType::Double:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE    );    // 10 : UA_Double    : double
		case QMetaType::QString:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_STRING    );    // 11 : UA_String    : { size_t length;  UA_Byte *data; }
		case QMetaType::QDateTime:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_DATETIME  );    // 12 : UA_DateTime  : int64_t
		case QMetaType::QUuid:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_GUID      );    // 13 : UA_Guid      : { UA_UInt32 data1; UA_UInt16 data2; UA_UInt16 data3; UA_Byte data4[8]; }
		case QMetaType::QByteArray:
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BYTESTRING);    // 14 : UA_ByteString : UA_String * A sequence of octets. */
		
		// TODO : QMetaType::QVariantList ???
		
		default:
			Q_ASSERT_X(false, "uaTypeNodeIdFromQType", "Unsupported datatype");
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);
		}
	}

	const UA_DataType * uaTypeFromQType(const QMetaType::Type & type)
	{
		switch (type)
		{
		case QMetaType::UnknownType:
			return &UA_TYPES[UA_TYPES_VARIANT];       // 23 : UA_Variant   : QVariant(QMetaType::UnknownType)
		case QMetaType::Bool:
			return &UA_TYPES[UA_TYPES_BOOLEAN];       // 0  : UA_Boolean   : bool
		case QMetaType::Char:
		case QMetaType::SChar:
			return &UA_TYPES[UA_TYPES_SBYTE];         // 1  : UA_SByte     : int8_t
		case QMetaType::UChar:
			return &UA_TYPES[UA_TYPES_BYTE];          // 2  : UA_Byte      : uint8_t
		case QMetaType::Short:
			return &UA_TYPES[UA_TYPES_INT16];         // 3  : UA_Int16     : int16_t
		case QMetaType::UShort:
			return &UA_TYPES[UA_TYPES_UINT16];        // 4  : UA_UInt16    : uint16_t
		case QMetaType::Int:
			return &UA_TYPES[UA_TYPES_INT32];         // 5  : UA_Int32     : int32_t
		case QMetaType::UInt:
			return &UA_TYPES[UA_TYPES_UINT32];        // 6  : UA_UInt32    : uint32_t
		case QMetaType::Long:
		case QMetaType::LongLong:
			return &UA_TYPES[UA_TYPES_INT64];         // 7  : UA_Int64     : int64_t
		case QMetaType::ULong:
		case QMetaType::ULongLong:
			return &UA_TYPES[UA_TYPES_UINT64];        // 8  : UA_UInt64    : uint64_t
		case QMetaType::Float:
			return &UA_TYPES[UA_TYPES_FLOAT];         // 9  : UA_Float     : float
		case QMetaType::Double:
			return &UA_TYPES[UA_TYPES_DOUBLE];        // 10 : UA_Double    : double 
		case QMetaType::QString:
			return &UA_TYPES[UA_TYPES_STRING];        // 11 : UA_String    : { size_t length;  UA_Byte *data; }
		case QMetaType::QDateTime:
			return &UA_TYPES[UA_TYPES_DATETIME];      // 12 : UA_DateTime  : int64_t
		case QMetaType::QUuid:
			return &UA_TYPES[UA_TYPES_GUID];          // 13 : UA_Guid      : { UA_UInt32 data1; UA_UInt16 data2; UA_UInt16 data3; UA_Byte data4[8]; }
		case QMetaType::QByteArray:
			return &UA_TYPES[UA_TYPES_BYTESTRING];    // 14 : UA_ByteString : UA_String * A sequence of octets. */
		default:
			Q_ASSERT_X(false, "uaTypeFromQType", "Unsupported datatype");
			return nullptr;
		}
	}

	UA_Variant uaVariantFromQVariant(const QVariant & var)
	{
		// TODO : support multidimentional arrays

		// get qt type
		QMetaType::Type qtType;
		const UA_DataType * uaType = nullptr;
		// fix qt type if array
		if (var.canConvert<QVariantList>())
		{
			qtType = QMetaType::QVariantList;
		}
		else
		{
            qtType = static_cast<QMetaType::Type>(var.type());
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
		case QMetaType::QVariantList:  // UA_Array
			return uaVariantFromQVariantArray(var);
		default:
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
		*ptr = UA_DATETIME_MSEC * (value.toMSecsSinceEpoch() - uaEpochStart.toMSecsSinceEpoch());
	}
	// specialization (QString)
	template<>
	void uaVariantFromQVariantScalar<UA_String, QString>(const QString &value, UA_String *ptr)
	{
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

	UA_Variant uaVariantFromQVariantArray(const QVariant & var)
	{
		// assume that the type of the first elem of the array is the type of all the array
		auto iter   = var.value<QSequentialIterable>();
        auto qtType = static_cast<QMetaType::Type>(iter.at(0).type());
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
			Q_ASSERT_X(false, "uaVariantFromQVariantArray", "Unsupported datatype");
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
        UA_Variant_setArray(&retVar, arr, static_cast<size_t>(iter.size()), type);
		// TODO : support multidimentional array
		retVar.arrayDimensions     = static_cast<UA_UInt32 *>(UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]));
        retVar.arrayDimensions[0]  = static_cast<UA_UInt32>(iter.size());
        retVar.arrayDimensionsSize = static_cast<size_t>(1);
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

    UA_Boolean UA_NodeId_equal_helper(const UA_NodeId *n1, const UA_NodeId n2)
    {
        return UA_NodeId_equal(n1, &n2);
    }

	QMetaType::Type uaTypeNodeIdToQType(const UA_NodeId * nodeId)
	{
		if (nodeId == nullptr) {
			return QMetaType::UnknownType;
		}
        if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE)))
		{
			return QMetaType::UnknownType;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN)))
		{
			return QMetaType::Bool;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE)))
		{
			return QMetaType::Char;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_BYTE)))
		{
			return QMetaType::UChar;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_INT16)))
		{
			return QMetaType::Short;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_UINT16)))
		{
			return QMetaType::UShort;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_INT32)))
		{
			return QMetaType::Int;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_UINT32)))
		{
			return QMetaType::UInt;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_INT64)))
		{
			return QMetaType::Long;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_UINT64)))
		{
			return QMetaType::ULong;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_FLOAT)))
		{
			return QMetaType::Float;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE)))
		{
			return QMetaType::Double;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_STRING)))
		{
			return QMetaType::QString;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_DATETIME)))
		{
			return QMetaType::QDateTime;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_GUID)))
		{
			return QMetaType::QUuid;
		}
        else if (UA_NodeId_equal_helper(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_BYTESTRING)))
		{
			return QMetaType::QByteArray;
		}
		else
		{
			Q_ASSERT_X(false, "uaTypeNodeIdToQType", "Unsupported datatype");
			return QMetaType::UnknownType;
		}
	}

	QMetaType::Type uaTypeToQType(const UA_DataType * uaType)
	{
		if (uaType == nullptr) {
			return QMetaType::UnknownType;
		}
		switch (uaType->typeIndex) {
		case UA_TYPES_VARIANT:
			return QMetaType::UnknownType;
		case UA_TYPES_BOOLEAN:
			return QMetaType::Bool;
		case UA_TYPES_SBYTE:
			return QMetaType::SChar;
		case UA_TYPES_BYTE:
			return QMetaType::UChar;
		case UA_TYPES_INT16:
			return QMetaType::Short;
		case UA_TYPES_UINT16:
			return QMetaType::UShort;
		case UA_TYPES_INT32:
			return QMetaType::Int;
		case UA_TYPES_UINT32:
			return QMetaType::UInt;
		case UA_TYPES_INT64:
			return QMetaType::LongLong;
		case UA_TYPES_UINT64:
			return QMetaType::ULongLong;
		case UA_TYPES_FLOAT:
			return QMetaType::Float;
		case UA_TYPES_DOUBLE:
			return QMetaType::Double;
		case UA_TYPES_STRING:
			return QMetaType::QString;	
		case UA_TYPES_DATETIME:
			return QMetaType::QDateTime;
		case UA_TYPES_GUID:
			return QMetaType::QUuid;
		case UA_TYPES_BYTESTRING:
			return QMetaType::QByteArray;
		default:
			Q_ASSERT_X(false, "uaTypeToQType", "Unsupported datatype");
			return QMetaType::UnknownType;
		}
	}

	QVariant uaVariantToQVariant(const UA_Variant & uaVariant)
	{
		// TODO : support multidimentional arrays

		if (uaVariant.type == nullptr) {
			return QVariant();
		}
		// first check if array
		if (!UA_Variant_isScalar(&uaVariant))
		{
			return uaVariantToQVariantArray(uaVariant);
		}
		// handle scalar
		switch (uaVariant.type->typeIndex) {
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
		default:
			Q_ASSERT_X(false, "uaVariantToQVariant", "Unsupported datatype");
			return QVariant();
		}
		return QVariant();
	}

	QVariant uaVariantToQVariantArray(const UA_Variant & uaVariant)
	{
		Q_ASSERT(!UA_Variant_isScalar(&uaVariant));
		if (uaVariant.type == nullptr) {
			return QVariant();
		}
		// handle array
		switch (uaVariant.type->typeIndex) {
		case UA_TYPES_VARIANT:
			return uaVariantToQVariantArray<QVariant   , UA_Variant   >(uaVariant, QMetaType::UnknownType);
		case UA_TYPES_BOOLEAN:
			return uaVariantToQVariantArray<bool       , UA_Boolean   >(uaVariant, QMetaType::Bool);
		case UA_TYPES_SBYTE:										   
			return uaVariantToQVariantArray<signed char, UA_SByte     >(uaVariant, QMetaType::SChar);
		case UA_TYPES_BYTE:											   
			return uaVariantToQVariantArray<uchar      , UA_Byte      >(uaVariant, QMetaType::UChar);
		case UA_TYPES_INT16:										   
			return uaVariantToQVariantArray<qint16     , UA_Int16     >(uaVariant, QMetaType::Short);
		case UA_TYPES_UINT16:										   
			return uaVariantToQVariantArray<quint16    , UA_UInt16    >(uaVariant, QMetaType::UShort);
		case UA_TYPES_INT32:										   
			return uaVariantToQVariantArray<qint32     , UA_Int32     >(uaVariant, QMetaType::Int);
		case UA_TYPES_UINT32:										   
			return uaVariantToQVariantArray<quint32    , UA_UInt32    >(uaVariant, QMetaType::UInt);
		case UA_TYPES_INT64:										   
			return uaVariantToQVariantArray<int64_t    , UA_Int64     >(uaVariant, QMetaType::LongLong);
		case UA_TYPES_UINT64:										   
			return uaVariantToQVariantArray<uint64_t   , UA_UInt64    >(uaVariant, QMetaType::ULongLong);
		case UA_TYPES_FLOAT:										   
			return uaVariantToQVariantArray<float      , UA_Float     >(uaVariant, QMetaType::Float);
		case UA_TYPES_DOUBLE:										   
			return uaVariantToQVariantArray<double     , UA_Double    >(uaVariant, QMetaType::Double);
		case UA_TYPES_STRING:										   
			return uaVariantToQVariantArray<QString    , UA_String    >(uaVariant, QMetaType::QString);
		case UA_TYPES_DATETIME:										   
			return uaVariantToQVariantArray<QDateTime  , UA_DateTime  >(uaVariant, QMetaType::QDateTime);
		case UA_TYPES_GUID:											   
			return uaVariantToQVariantArray<QUuid      , UA_Guid      >(uaVariant, QMetaType::QUuid);
		case UA_TYPES_BYTESTRING:
			return uaVariantToQVariantArray<QByteArray , UA_ByteString>(uaVariant, QMetaType::QByteArray);
		default:
			Q_ASSERT_X(false, "uaVariantToQVariantArray", "Unsupported datatype");
			return QVariant();
		}
		return QVariant();
	}

	template<typename TARGETTYPE, typename UATYPE>
	QVariant uaVariantToQVariantArray(const UA_Variant & var, QMetaType::Type type)
	{
		// if empty
		if (var.arrayLength == 0 && var.data == UA_EMPTY_ARRAY_SENTINEL)
		{
			return QVariantList();
		}
		// get start of array
		UATYPE *temp = static_cast<UATYPE *>(var.data);
		// copy array data
		QVariantList list;
		for (size_t i = 0; i < var.arrayLength; i++) 
		{
			QVariant tempVar = QVariant::fromValue(uaVariantToQVariantScalar<TARGETTYPE, UATYPE>(&temp[i]));
			if (type != QMetaType::UnknownType && type != static_cast<QMetaType::Type>(tempVar.type()))
			{
				tempVar.convert(type);
			}
			list.append(tempVar);
		}
		// return variant list
		return list;
	}

	template<typename TARGETTYPE, typename UATYPE>
	QVariant uaVariantToQVariantScalar(const UA_Variant & uaVariant, QMetaType::Type type)
	{
		UATYPE  *temp    = static_cast<UATYPE *>(uaVariant.data);
		QVariant tempVar = QVariant::fromValue(uaVariantToQVariantScalar<TARGETTYPE, UATYPE>(temp));
		if (type != QMetaType::UnknownType && type != static_cast<QMetaType::Type>(tempVar.type()))
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
		return QVariant((QVariant::Type)QMetaType::UnknownType);
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
		const QDateTime epochStart(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC);
		return epochStart.addMSecs(*data / UA_DATETIME_MSEC).toLocalTime();
	}
	// specialization (QUuid)
	template<>
	QUuid uaVariantToQVariantScalar<QUuid, UA_Guid>(const UA_Guid *data)
	{
		return QUuid(data->data1, data->data2, data->data3, data->data4[0], data->data4[1], data->data4[2],
			data->data4[3], data->data4[4], data->data4[5], data->data4[6], data->data4[7]);
	}

}

QT_END_NAMESPACE
