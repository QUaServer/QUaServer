#ifndef QUATYPESCONVERTER_H
#define QUATYPESCONVERTER_H

#include <QUaCustomDataTypes>

QT_BEGIN_NAMESPACE

namespace QUaTypesConverter {

	// common convertions
	UA_NodeId nodeIdFromQString  (const QString &name);
	QString   nodeIdToQString    (const UA_NodeId &id);
	
	bool      nodeIdStringSplit  (const QString &nodeIdString, quint16 *nsIndex, QString *identifier, char *identifierType);
	QString   nodeClassToQString (const UA_NodeClass &nclass);
	
	QString   uaStringToQString  (const UA_String &string);
	UA_String uaStringFromQString(const QString &uaString);

	// qt supported
	bool            isQTypeArray    (const QMetaType::Type &type);
	QMetaType::Type getQArrayType   (const QMetaType::Type &type);
	bool            isSupportedQType(const QMetaType::Type &type);
	// ua from c++
	template<typename T>
	UA_NodeId uaTypeNodeIdFromCpp();
	// qt from c++
	template<typename T>
	QMetaType::Type qtTypeFromCpp();
	// ua from qt
	UA_NodeId          uaTypeNodeIdFromQType(const QMetaType::Type &type);
	const UA_DataType *uaTypeFromQType      (const QMetaType::Type &type);
	UA_Variant         uaVariantFromQVariant(
		const QVariant &var
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
#ifndef OPEN62541_ISSUE3934_RESOLVED
		, const UA_DataType * optDataType = nullptr
#endif // !OPEN62541_ISSUE3934_RESOLVED
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
	);
	// ua from qt : scalar
	template<typename TARGETTYPE, typename QTTYPE> // has specializations
	UA_Variant uaVariantFromQVariantScalar(const QVariant &var, const UA_DataType *type);
	template<> // TODO : implement better
	UA_Variant uaVariantFromQVariantScalar<UA_Variant, QVariant>(const QVariant & var, const UA_DataType * type);
	template<typename TARGETTYPE, typename QTTYPE> // has specializations
	void       uaVariantFromQVariantScalar(const QTTYPE &var, TARGETTYPE *ptr);
	// ua from qt : array
	UA_Variant uaVariantFromQVariantArray(const QVariant & var);
	template<typename TARGETTYPE, typename QTTYPE>
	UA_Variant uaVariantFromQVariantArray(const QVariant &var, const UA_DataType *type);
	template<> // TODO : implement better
	UA_Variant uaVariantFromQVariantArray<UA_Variant, QVariant>(const QVariant & var, const UA_DataType * type);

	// ua to qt : array
	enum class ArrayType
	{
		QList   = 0,
		QVector = 1,
		Invalid = 2
	};
	// ua to qt
	QMetaType::Type uaTypeNodeIdToQType(const UA_NodeId   *nodeId   );
	QMetaType::Type uaTypeToQType      (const UA_DataType *uaType   );
	QVariant        uaVariantToQVariant(const UA_Variant  &uaVariant, const ArrayType& arrType = ArrayType::QList);
	// ua to qt : scalar
	template<typename TARGETTYPE, typename UATYPE> // has specializations
	QVariant   uaVariantToQVariantScalar(const UA_Variant &uaVariant, QMetaType::Type type);
	// TODO
	template<>
	QVariant uaVariantToQVariantScalar<QVariant, UA_Variant>(const UA_Variant & uaVariant, QMetaType::Type type);
	template<typename TARGETTYPE, typename UATYPE> // has specializations
	TARGETTYPE uaVariantToQVariantScalar(const UATYPE *data);
	QVariant uaVariantToQVariantArray (const UA_Variant &uaVariant, 
		                               const ArrayType  &arrType = ArrayType::QList);
	QVariant uaVariantToQVariantList  (const UA_Variant &uaVariant);
	QVariant uaVariantToQVariantVector(const UA_Variant &uaVariant);
    template <typename ARRAYTYPE, typename UATYPE>
	QVariant uaVariantToQVariantArray (const UA_Variant &var, QMetaType::Type type);

	template<typename T>
	UA_NodeId uaTypeNodeIdFromCpp()
	{
		if (std::is_same<T, QVariant>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE);
		}
		else if (std::is_same<T, bool>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BOOLEAN);
		}
		else if (std::is_same<T, char>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_SBYTE);
		}
		else if (std::is_same<T, uchar>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BYTE);
		}
		else if (std::is_same<T, qint16>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_INT16);
		}
		else if (std::is_same<T, quint16>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_UINT16);
		}
		else if (std::is_same<T, int>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
		}
		else if (std::is_same<T, qint32>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_INT32);
		}
		else if (std::is_same<T, quint32>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_UINT32);
		}
		else if (std::is_same<T, int64_t>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_INT64);
		}
		else if (std::is_same<T, uint64_t>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_UINT64);
		}
		else if (std::is_same<T, float>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_FLOAT);
		}
		else if (std::is_same<T, double>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE);
		}
		else if (std::is_same<T, QString>::value || std::is_same<T, const char *>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
		}
		else if (std::is_same<T, QDateTime>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_DATETIME);
		}
		else if (std::is_same<T, QUuid>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_GUID);
		}
		else if (std::is_same<T, QByteArray>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_BYTESTRING);
		}
		else if (std::is_same<T, QUaNodeId>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_NODEID);
		}
		else if (std::is_same<T, QUaStatusCode>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_STATUSCODE);
		}
		else if (std::is_same<T, QUaQualifiedName>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_QUALIFIEDNAME);
		}
		else if (std::is_same<T, QUaLocalizedText>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_LOCALIZEDTEXT);
		}
		// TODO : image
		//else if (std::is_same<T, QImage>::value)
		//{
		//	return UA_NODEID_NUMERIC(0, UA_NS0ID_IMAGE);
		//}
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
		else if (std::is_same<T, QUaOptionSet>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_OPTIONSET);
		}
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		else if (std::is_same<T, QTimeZone>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_TIMEZONEDATATYPE);
		}
		else if (std::is_same<T, QUaChangeStructureDataType>::value)
		{
			return UA_NODEID_NUMERIC(0, UA_NS0ID_MODELCHANGESTRUCTUREDATATYPE);
		}
#endif
		Q_ASSERT_X(false, "uaTypeNodeIdFromCpp", "Unsupported type");
		return UA_NodeId();
	}

	template<typename T>
	QMetaType::Type qtTypeFromCpp()
	{
		if (std::is_same<T, QVariant>::value)
		{
			return QMetaType::UnknownType;
		}
		else if (std::is_same<T, bool>::value)
		{
			return QMetaType::Bool;
		}
		else if (std::is_same<T, char>::value)
		{
			return QMetaType::Char;
		}
		else if (std::is_same<T, uchar>::value)
		{
			return QMetaType::UChar;
		}
		else if (std::is_same<T, qint16>::value)
		{
			return QMetaType::Short;
		}
		else if (std::is_same<T, quint16>::value)
		{
			return QMetaType::UShort;
		}
		else if (std::is_same<T, int>::value)
		{
			return QMetaType::Int;
		}
		else if (std::is_same<T, qint32>::value)
		{
			return QMetaType::Int;
		}
		else if (std::is_same<T, quint32>::value)
		{
			return QMetaType::UInt;
		}
		else if (std::is_same<T, int64_t>::value)
		{
			return QMetaType::LongLong;
		}
		else if (std::is_same<T, uint64_t>::value)
		{
			return QMetaType::ULongLong;
		}
		else if (std::is_same<T, float>::value)
		{
			return QMetaType::Float;
		}
		else if (std::is_same<T, double>::value)
		{
			return QMetaType::Double;
		}
		else if (std::is_same<T, QString>::value)
		{
			return QMetaType::QString;
		}
		else if (std::is_same<T, QDateTime>::value)
		{
			return QMetaType::QDateTime;
		}
		else if (std::is_same<T, QUuid>::value)
		{
			return QMetaType::QUuid;
		}
		else if (std::is_same<T, QByteArray>::value)
		{
			return QMetaType::QByteArray;
		}
		else if (std::is_same<T, QVariantList>::value)
		{
			return QMetaType::QVariantList;
		}
		else if (std::is_same<T, QUaNodeId>::value)
		{
			return QMetaType_NodeId;
		}
		else if (std::is_same<T, QUaStatusCode>::value)
		{
			return QMetaType_StatusCode;
		}
		else if (std::is_same<T, QUaQualifiedName>::value)
		{
			return QMetaType_QualifiedName;
		}
		else if (std::is_same<T, QUaLocalizedText>::value)
		{
			return QMetaType_LocalizedText;
		}
		// TODO : image
		//else if (std::is_same<T, QImage>::value)
		//{
		//	return QMetaType_Image;
		//}
#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL
		else if (std::is_same<T, QUaOptionSet>::value)
		{
			return QMetaType_OptionSet;
		}
#endif // UA_GENERATED_NAMESPACE_ZERO_FULL
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
		else if (std::is_same<T, QTimeZone>::value)
		{
			return QMetaType_TimeZone;
		}
		else if (std::is_same<T, QUaChangeStructureDataType>::value)
		{
			return QMetaType_ChangeStructureDataType;
		}
#endif
		Q_ASSERT_X(false, "qtTypeFromCpp", "Unsupported type");
		return QMetaType::UnknownType;
	}

	void registerCustomTypes();
}

QT_END_NAMESPACE

#endif // QUATYPESCONVERTER_H
