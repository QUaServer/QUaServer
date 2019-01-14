#include "qopcuaservernode.h"

#include <QUuid>
#include <QRegularExpression>
#include <QDebug>

QOpcUaServerNode::QOpcUaServerNode(QOpcUaServerNode *parent) : QObject(parent)
{
	m_qopcuaserver = parent->m_qopcuaserver;
	m_nodeId       = UA_NODEID_NULL;
	// NOTE : type NodeId is null until class is registered in server
	Q_CHECK_PTR(m_qopcuaserver);
}

QOpcUaServerNode::QOpcUaServerNode(QOpcUaServer * server) : QObject(server)
{
	m_qopcuaserver = server;
	m_nodeId       = UA_NODEID_NULL;
	// NOTE : type NodeId is null until class is registered in server
	Q_CHECK_PTR(m_qopcuaserver);
}

QString QOpcUaServerNode::get_displayName() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read description
	UA_LocalizedText outDisplayName;
	UA_Server_readDisplayName(m_qopcuaserver->m_server, m_nodeId, &outDisplayName);
	return QOpcUaServerNode::uaStringToQString(outDisplayName.text);
	// TODO : handle outDisplayName.locale
}

void QOpcUaServerNode::set_displayName(const QString & displayName)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDisplayName = displayName.toUtf8(); // NOTE : QByteArray must exist in stack
	UA_LocalizedText uaDisplayName = UA_LOCALIZEDTEXT((char*)"", byteDisplayName.data());
	// set value
	UA_Server_writeDisplayName(m_qopcuaserver->m_server, m_nodeId, uaDisplayName);
	// TODO : handle locale
}

QString QOpcUaServerNode::get_description() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read description
	UA_LocalizedText outDescription;
	UA_Server_readDescription(m_qopcuaserver->m_server, m_nodeId, &outDescription);
	return QOpcUaServerNode::uaStringToQString(outDescription.text); 
	// TODO : handle outDescription.locale
}

void QOpcUaServerNode::set_description(const QString & description)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_LocalizedText
	QByteArray byteDescription = description.toUtf8(); // NOTE : QByteArray must exist in stack
	UA_LocalizedText uaDescription = UA_LOCALIZEDTEXT((char*)"", byteDescription.data());
	// set value
	UA_Server_writeDescription(m_qopcuaserver->m_server, m_nodeId, uaDescription);
	// TODO : handle locale
}

quint32 QOpcUaServerNode::get_writeMask() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return quint32();
	}
	// read writeMask
	UA_UInt32 outWriteMask;
	UA_Server_readWriteMask(m_qopcuaserver->m_server, m_nodeId, &outWriteMask);
	return outWriteMask;
}

void QOpcUaServerNode::set_writeMask(const quint32 & writeMask)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// set value
	UA_Server_writeWriteMask(m_qopcuaserver->m_server, m_nodeId, writeMask);
}

QString QOpcUaServerNode::get_nodeId() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	return QOpcUaServerNode::nodeIdToQString(m_nodeId);
}

QString QOpcUaServerNode::get_nodeClass() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QString();
	}
	// read nodeClass
	UA_NodeClass outNodeClass;
	UA_Server_readNodeClass(m_qopcuaserver->m_server, m_nodeId, &outNodeClass);
	// convert to QString
	return QOpcUaServerNode::nodeClassToQString(outNodeClass);
}

QPair<quint16, QString> QOpcUaServerNode::get_browseName() const
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	if (UA_NodeId_isNull(&m_nodeId))
	{
		return QPair<quint16, QString>();
	}
	// read browse name
	UA_QualifiedName outBrowseName;
	UA_Server_readBrowseName(m_qopcuaserver->m_server, m_nodeId, &outBrowseName);
	// populate return value
	return QPair<quint16, QString>(outBrowseName.namespaceIndex, QOpcUaServerNode::uaStringToQString(outBrowseName.name));
}

void QOpcUaServerNode::set_browseName(const QPair<quint16, QString> & browseName)
{
	Q_CHECK_PTR(m_qopcuaserver);
	Q_ASSERT(!UA_NodeId_isNull(&m_nodeId));
	// convert to UA_QualifiedName
	UA_QualifiedName bName;
	bName.namespaceIndex = browseName.first;
	bName.name           = QOpcUaServerNode::uaStringFromQString(browseName.second);
	// set value
	UA_Server_writeBrowseName(m_qopcuaserver->m_server, m_nodeId, bName);
}

UA_NodeId QOpcUaServerNode::nodeIdFromQString(const QString & name)
{
	quint16 namespaceIndex;
	QString identifierString;
	char    identifierType;
	bool success = QOpcUaServerNode::nodeIdStringSplit(name, &namespaceIndex, &identifierString, &identifierType);

	if (!success) {
		qWarning() << "Failed to split node id string:" << name;
		return UA_NODEID_NULL;
	}

	switch (identifierType) {
	case 'i': {
		bool isNumber;
		uint identifier = identifierString.toUInt(&isNumber);
		if (isNumber && identifier <= ((std::numeric_limits<quint32>::max)()))
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

QString QOpcUaServerNode::nodeIdToQString(const UA_NodeId & id)
{
	QString result = QString::fromLatin1("ns=%1;").arg(id.namespaceIndex);

	switch (id.identifierType) {
	case UA_NODEIDTYPE_NUMERIC:
		result.append(QString::fromLatin1("i=%1").arg(id.identifier.numeric));
		break;
	case UA_NODEIDTYPE_STRING:
		result.append(QLatin1String("s="));
		result.append(QString::fromLocal8Bit(reinterpret_cast<char *>(id.identifier.string.data), id.identifier.string.length));
		break;
	case UA_NODEIDTYPE_GUID: {
		const UA_Guid &src = id.identifier.guid;
		const QUuid uuid(src.data1, src.data2, src.data3, src.data4[0], src.data4[1], src.data4[2],
			src.data4[3], src.data4[4], src.data4[5], src.data4[6], src.data4[7]);
		result.append(QStringLiteral("g=")).append(uuid.toString().midRef(1, 36)); // Remove enclosing {...}
		break;
	}
	case UA_NODEIDTYPE_BYTESTRING: {
		const QByteArray temp(reinterpret_cast<char *>(id.identifier.byteString.data), id.identifier.byteString.length);
		result.append(QStringLiteral("b=")).append(temp.toBase64());
		break;
	}
	default:
		qWarning() << "Open62541 Utils: Could not convert UA_NodeId to QString";
		result.clear();
	}
	return result;
}

bool QOpcUaServerNode::nodeIdStringSplit(const QString & nodeIdString, quint16 * nsIndex, QString * identifier, char * identifierType)
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

QString QOpcUaServerNode::nodeClassToQString(const UA_NodeClass & nclass)
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

QString QOpcUaServerNode::uaStringToQString(const UA_String & string)
{
	return QString::fromLocal8Bit(reinterpret_cast<char *>(string.data), string.length);
}

UA_String QOpcUaServerNode::uaStringFromQString(const QString & uaString)
{
	return UA_STRING_ALLOC(uaString.toUtf8().constData());
}



