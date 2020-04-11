#include "quacustomdatatypes.h"

#include <QUaTypesConverter>

#include<QMetaEnum>

QMetaEnum QUaDataType::m_metaEnum = QMetaEnum::fromType<QUa::Type>();

QUaDataType::QUaDataType()
{
	m_type = QUa::Type::UnknownType;
}

QUaDataType::QUaDataType(const QMetaType::Type& metaType)
{
	m_type = static_cast<QUa::Type>(metaType);
}

QUaDataType::QUaDataType(const QString& strType)
{
    *this = QUaDataType(strType.toUtf8());
}

QUaDataType::QUaDataType(const QByteArray& byteType)
{
	bool ok = false;
	int val = m_metaEnum.keyToValue(byteType.constData(), &ok);
    m_type  = static_cast<QUa::Type>(val);
}

QUaDataType::operator QMetaType::Type() const
{
	return static_cast<QMetaType::Type>(m_type);
}

QUaDataType::operator QString() const
{
	return QString(m_metaEnum.valueToKey(static_cast<int>(m_type)));
}

bool QUaDataType::operator==(const QMetaType::Type& metaType)
{
	return static_cast<QMetaType::Type>(m_type) == metaType;
}

void QUaDataType::operator=(const QString& strType)
{
	*this = QUaDataType(strType.toUtf8());
}

QMetaEnum QUaStatusCode::m_metaEnum = QMetaEnum::fromType<QUa::Status>();

// init static hash
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

bool QUaStatusCode::operator==(const QUaStatus& uaStatus)
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
	m_name = strXmlQualName;
	QStringList components = strXmlQualName.split(QLatin1String(";"));
	// check if valid xml format
	if (components.size() != 2)
	{
		// if no valid xml format, assume ns = 0 and given string is name
		return;
	}
	// check if valid namespace found, else assume ns = 0 and given string is name
	quint16 new_ns;
	if (components.at(0).contains(QRegularExpression(QLatin1String("^ns=[0-9]+")))) {
		bool success = false;
		uint ns = components.at(0).midRef(3).toString().toUInt(&success);
		if (!success || ns > (std::numeric_limits<quint16>::max)())
		{
			return;
		}
		new_ns = ns;
	}
	// check if valid name found, else assume ns = 0 and given string is name
	if (!components.last().contains(QRegularExpression(QLatin1String("^[isgb]="))))
	{
		return;
	}
	// if reached here, xml format is correct
	m_namespace = new_ns;
	m_name = components.last().midRef(2).toString();
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

quint16 QUaQualifiedName::namespaceIndex() const
{
	return m_namespace;
}

void QUaQualifiedName::setNamespaceIndex(const quint16& namespaceIndex)
{
	m_namespace = namespaceIndex;
}

QString QUaQualifiedName::name() const
{
	return m_name;
}

void QUaQualifiedName::seName(const QString& name)
{
	m_name = name;
}

QString QUaQualifiedName::toXmlString() const
{
	// use ::operator QString()
	return *this;
}

UA_QualifiedName QUaQualifiedName::toUaQualifiedName()
{
	// use ::operator UA_QualifiedName()
	return *this;
}

QUaQualifiedName QUaQualifiedName::fromXmlString(const QString& strXmlQualName)
{
	return QUaQualifiedName(strXmlQualName);
}

QUaQualifiedName QUaQualifiedName::fromUaQualifiedName(const UA_QualifiedName& uaQualName)
{
	return QUaQualifiedName(uaQualName);
}

QUaChangeStructureDataType::QUaChangeStructureDataType()
	: m_strNodeIdAffected(""),
	m_strNodeIdAffectedType(""),
	m_uiVerb(QUaChangeVerb::NodeAdded)
{
}

QUaChangeStructureDataType::QUaChangeStructureDataType(const QString& strNodeIdAffected, const QString& strNodeIdAffectedType, const Verb& uiVerb)
	: m_strNodeIdAffected(strNodeIdAffected),
	m_strNodeIdAffectedType(strNodeIdAffectedType),
	m_uiVerb(uiVerb)
{
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
