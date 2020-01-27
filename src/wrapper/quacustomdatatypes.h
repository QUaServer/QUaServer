#ifndef QUACUSTOMDATATYPES_H
#define QUACUSTOMDATATYPES_H

#include <QObject>
#include <QVariant>
#include <QUuid>
#include <QRegularExpression>
#include <QDate>
#include <QTimeZone>
#include <QDebug>

#include <open62541.h>

Q_DECLARE_METATYPE(QTimeZone);

#define METATYPE_OFFSET_LOCALIZEDTEXT 1
#define METATYPE_LOCALIZEDTEXT (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_LOCALIZEDTEXT)

#define METATYPE_OFFSET_TIMEZONEDATATYPE 2
#define METATYPE_TIMEZONEDATATYPE (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_TIMEZONEDATATYPE)

#define METATYPE_OFFSET_NODEID 3
#define METATYPE_NODEID (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_NODEID)

#define METATYPE_OFFSET_CHANGESTRUCTUREDATATYPE 4
#define METATYPE_CHANGESTRUCTUREDATATYPE (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_CHANGESTRUCTUREDATATYPE)

#define METATYPE_OFFSET_IMAGE 5
#define METATYPE_IMAGE (QMetaType::Type)(QMetaType::User + METATYPE_OFFSET_IMAGE)

struct QUaChangeStructureDataType
{
	enum Verb
	{
		NodeAdded        = 1,
		NodeDeleted      = 2,
		ReferenceAdded   = 4,
		ReferenceDeleted = 8,
		DataTypeChanged  = 16
	};
	QUaChangeStructureDataType();
	QUaChangeStructureDataType(const QString &strNodeIdAffected, const QString &strNodeIdAffectedType, const Verb &uiVerb);

	QString m_strNodeIdAffected;
	QString m_strNodeIdAffectedType;
	uchar   m_uiVerb;
};
typedef QUaChangeStructureDataType::Verb QUaChangeVerb;

inline bool operator==(const QUaChangeStructureDataType& lhs, const QUaChangeStructureDataType& rhs) 
{
	return lhs.m_strNodeIdAffected    .compare(rhs.m_strNodeIdAffected    , Qt::CaseInsensitive) == 0 &&
		   lhs.m_strNodeIdAffectedType.compare(rhs.m_strNodeIdAffectedType, Qt::CaseInsensitive) == 0 &&
		   lhs.m_uiVerb == rhs.m_uiVerb;
}

Q_DECLARE_METATYPE(QUaChangeStructureDataType);

#endif // QUACUSTOMDATATYPES_H
