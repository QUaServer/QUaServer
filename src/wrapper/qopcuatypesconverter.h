#ifndef QOPCUATYPESCONVERTER_H
#define QOPCUATYPESCONVERTER_H

#include <QObject>
#include <QVariant>
#include <QUuid>
#include <QRegularExpression>
#include <QDate>
#include <QDebug>

#include "open62541.h"

QT_BEGIN_NAMESPACE

namespace QOpcUaTypesConverter {

	// common convertions
	UA_NodeId nodeIdFromQString  (const QString &name);
	QString   nodeIdToQString    (const UA_NodeId &id);
	
	bool      nodeIdStringSplit  (const QString &nodeIdString, quint16 *nsIndex, QString *identifier, char *identifierType);
	QString   nodeClassToQString (const UA_NodeClass &nclass);
	
	QString   uaStringToQString  (const UA_String &string);
	UA_String uaStringFromQString(const QString &uaString);

	// ua from qt
	UA_NodeId          uaTypeNodeIdFromQType(const QMetaType::Type &type);
	const UA_DataType *uaTypeFromQType      (const QMetaType::Type &type);
	UA_Variant         uaVariantFromQVariant(const QVariant        & var);
	// ua from qt : scalar
	template<typename TARGETTYPE, typename QTTYPE>
	UA_Variant uaVariantFromQVariantScalar(const QVariant &var, const UA_DataType *type);
	template<typename TARGETTYPE, typename QTTYPE> // has specializations
	void       uaVariantFromQVariantScalar(const QTTYPE &var, TARGETTYPE *ptr);
	// ua from qt : array
	UA_Variant uaVariantFromQVariantArray(const QVariant & var);
	template<typename TARGETTYPE, typename QTTYPE>
	UA_Variant uaVariantFromQVariantArray(const QVariant &var, const UA_DataType *type);

	// ua to qt
	QMetaType::Type uaTypeNodeIdToQType(const UA_NodeId   *nodeId   );
	QMetaType::Type uaTypeToQType      (const UA_DataType *uaType   );
	QVariant        uaVariantToQVariant(const UA_Variant  &uaVariant);
	// ua to qt : scalar
	template<typename TARGETTYPE, typename UATYPE>
	QVariant   uaVariantToQVariantScalar(const UA_Variant &uaVariant, QMetaType::Type type);
	template<typename TARGETTYPE, typename UATYPE> // has specializations
	TARGETTYPE uaVariantToQVariantScalar(const UATYPE *data);
	// ua to qt : array
	QVariant uaVariantToQVariantArray(const UA_Variant  &uaVariant);
	template<typename TARGETTYPE, typename UATYPE>
	QVariant uaVariantToQVariantArray(const UA_Variant &var, QMetaType::Type type);

}

QT_END_NAMESPACE

#endif // QOPCUATYPESCONVERTER_H
