#ifndef QUATYPESCONVERTER_H
#define QUATYPESCONVERTER_H

#include <QObject>
#include <QVariant>
#include <QUuid>
#include <QRegularExpression>
#include <QDate>
#include <QTimeZone>
#include <QDebug>

#include "open62541.h"

Q_DECLARE_METATYPE(QTimeZone);

#define METATYPE_OFFSET_LOCALIZEDTEXT 1
#define METATYPE_LOCALIZEDTEXT (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_LOCALIZEDTEXT)

#define METATYPE_OFFSET_TIMEZONEDATATYPE 2
#define METATYPE_TIMEZONEDATATYPE (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_TIMEZONEDATATYPE)

#define METATYPE_OFFSET_NODEID 3
#define METATYPE_NODEID (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_NODEID)

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
	bool isSupportedQType(const QMetaType::Type &type);
	// ua from c++
	template<typename T>
	UA_NodeId uaTypeNodeIdFromCpp();
	// qt from c++
	template<typename T>
	QMetaType::Type qtTypeFromCpp();
	// ua from qt
	UA_NodeId          uaTypeNodeIdFromQType(const QMetaType::Type &type);
	const UA_DataType *uaTypeFromQType      (const QMetaType::Type &type);
	UA_Variant         uaVariantFromQVariant(const QVariant        & var, QMetaType::Type qtType = QMetaType::UnknownType);
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

	// ua to qt
	QMetaType::Type uaTypeNodeIdToQType(const UA_NodeId   *nodeId   );
	QMetaType::Type uaTypeToQType      (const UA_DataType *uaType   );
	QVariant        uaVariantToQVariant(const UA_Variant  &uaVariant);
	// ua to qt : scalar
	template<typename TARGETTYPE, typename UATYPE> // has specializations
	QVariant   uaVariantToQVariantScalar(const UA_Variant &uaVariant, QMetaType::Type type);
	// TODO
	template<>
	QVariant uaVariantToQVariantScalar<QVariant, UA_Variant>(const UA_Variant & uaVariant, QMetaType::Type type);
	template<typename TARGETTYPE, typename UATYPE> // has specializations
	TARGETTYPE uaVariantToQVariantScalar(const UATYPE *data);
	// ua to qt : array
	QVariant uaVariantToQVariantArray(const UA_Variant  &uaVariant);
	template<typename TARGETTYPE, typename UATYPE> // has specializations
	QVariant uaVariantToQVariantArray(const UA_Variant &var, QMetaType::Type type);


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
		// TODO : ?
		//else if (std::is_same<T, QVariantList>::value)
		//{
		//	return ;
		//}
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
		Q_ASSERT_X(false, "qtTypeFromCpp", "Unsupported type");
		return QMetaType::UnknownType;
	}

}

QT_END_NAMESPACE

#endif // QUATYPESCONVERTER_H
