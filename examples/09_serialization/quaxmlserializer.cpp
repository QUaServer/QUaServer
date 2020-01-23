#include "quaxmlserializer.h"

QUaXmlSerializer::QUaXmlSerializer()
{
	this->reset();
}

void QUaXmlSerializer::reset()
{
	// reset serialization state
	m_doc.clear();
	QDomProcessingInstruction header = m_doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	m_doc.appendChild(header);
	QDomElement root = m_doc.createElement("nodes");
	m_doc.appendChild(root);
	// reset deserialization state
	m_mapNodeData.clear();
}

QByteArray QUaXmlSerializer::toByteArray() const
{
	return m_doc.toByteArray();
}

bool QUaXmlSerializer::fromByteArray(const QUaServer* server, const QByteArray& xmlData, QString& strError)
{
	// load from xml
	int line, col;
	m_doc.setContent(xmlData, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		strError = QObject::tr("Error : Invalid XML in Line %1 Column %2 Error %3").arg(line).arg(col).arg(strError);
		return false;
	}
	// fill up nodes data
	QDomElement root = m_doc.documentElement();
	QDomNode iter = root.firstChild();
	while (!iter.isNull()) 
	{
		// try to convert the node to an element, continue if not possible
		QDomElement node = iter.toElement(); 
		if (node.isNull()) 
		{
			iter = iter.nextSibling();
			continue;
		}
		// parse nodeId
		QString nodeId = this->readNodeIdAttribute(node, strError);
		if (nodeId.isEmpty())
		{
			iter = iter.nextSibling();
			continue;
		}
		// parse typeName
		QString typeName = this->readTypeNameAttribute(server, node, strError);
		if (typeName.isEmpty())
		{
			iter = iter.nextSibling();
			continue;
		}
		// read rest of attributes
		QMap<QString, QVariant> mapAttrs;
		auto attrs = node.attributes();
		for (int i = 0; i < attrs.count(); i++)
		{
			QDomAttr attr = attrs.item(i).toAttr();
			if (attr.isNull() || 
				attr.name().compare("nodeId") == 0 ||
				attr.name().compare("typeName") == 0)
			{
				continue;
			}
			// deserialize
			QString name = attr.name();
			QVariant value = this->readAttribute(attr.value(), strError);
			if (!value.isValid())
			{
				continue;
			}
			Q_ASSERT(!mapAttrs.contains(name));
			mapAttrs.insert(name, value);
		}

		// TODO : parse references
		QList<QUaForwardReference> refs;

		// insert to internal data
		m_mapNodeData.insert(nodeId, {
			typeName,
			mapAttrs,
			refs
		});
		// continue with next element
		iter = iter.nextSibling();
	}
	return true;
}

bool QUaXmlSerializer::writeInstance(
	const QString& nodeId, 
	const QString& typeName, 
	const QMap<QString, QVariant>& attrs, 
	const QList<QUaForwardReference>& forwardRefs,
	QQueue<QUaLog>& logOut)
{
	Q_UNUSED(logOut);
	QDomElement root = m_doc.documentElement();
	// create node in xml
	QDomElement node = m_doc.createElement("n");
	root.appendChild(node);
	// copy attributes
	this->writeAttribute(node, "nodeId"  , nodeId);
	this->writeAttribute(node, "typeName", typeName);
	auto i = attrs.constBegin();
	while (i != attrs.constEnd())
	{
		this->writeAttribute(node, i.key(), i.value());
		i++;
	}
	// copy references
	for (auto &ref : forwardRefs)
	{
		QDomElement refElem = m_doc.createElement("r");
		node.appendChild(refElem);
		this->writeAttribute(refElem, "nodeIdTarget", ref.nodeIdTarget);
		this->writeAttribute(refElem, "targetType"  , ref.targetType);
		this->writeAttribute(refElem, "forwardName" , ref.refType.strForwardName);
		this->writeAttribute(refElem, "inverseName" , ref.refType.strInverseName);
	}
	return true;
}

bool QUaXmlSerializer::readInstance(
	const QString& nodeId, 
	QString& typeName, 
	QMap<QString, QVariant>& attrs, 
	QList<QUaForwardReference>& forwardRefs,
	QQueue<QUaLog>& logOut)
{
	if (!m_mapNodeData.contains(nodeId))
	{
		logOut.append({
			QObject::tr("Could not find nodeId %1").arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// set return values
	auto& data  = m_mapNodeData[nodeId];
	typeName    = data.typeName;
	attrs       = data.attrs;
	forwardRefs = data.forwardRefs;
	return true;
}

void QUaXmlSerializer::writeAttribute(
	QDomElement& node, 
	const QString& strName,
	const QVariant& varValue
)
{
	auto type = static_cast<QMetaType::Type>(varValue.type());
	if (type == QMetaType::UChar)
	{
		node.setAttribute(strName, QString("%1").arg(varValue.toUInt()));
		return;
	}
	node.setAttribute(strName, varValue.toString());
}

QString QUaXmlSerializer::readNodeIdAttribute(QDomElement& node, QString& strError)
{
	if (!node.hasAttribute("nodeId"))
	{
		strError += QObject::tr("Error : Found node element without nodeId attribute. Ignoring.");
		return "";
	}
	QString nodeId = node.attribute("nodeId", "");
	if (nodeId.isEmpty())
	{
		strError += QObject::tr("Error : Found node element with empty nodeId attribute. Ignoring.");
		return "";
	}
	if (!QUaServer::isIdValid(nodeId))
	{
		strError += QObject::tr("Error : Found node element with invalid nodeId attribute %1. Ignoring.").arg(nodeId);
		return "";
	}
	// success
	return nodeId;
}

QString QUaXmlSerializer::readTypeNameAttribute(const QUaServer* server, QDomElement& node, QString& strError)
{
	if (!node.hasAttribute("typeName"))
	{
		strError += QObject::tr("Error : Found node element without typeName attribute. Ignoring.");
		return "";
	}
	QString typeName = node.attribute("typeName", "");
	if (typeName.isEmpty())
	{
		strError += QObject::tr("Error : Found node element with empty typeName attribute. Ignoring.");
		return "";
	}
	if (!server->isTypeNameRegistered(typeName))
	{
		strError += QObject::tr("Error : Found node element with unregistered typeName attribute %1. Ignoring.").arg(typeName);
		return "";
	}
	// success
	return typeName;
}

QVariant QUaXmlSerializer::readAttribute(const QString& strValue, QString& strError)
{
	// first try int
	bool ok;
	int intVal = strValue.toInt(&ok);
	if (ok)
	{
		return intVal;
	}
	// then try double
	double dblVal = strValue.toDouble(&ok);
	if (ok)
	{
		return dblVal;
	}
	// if all fails, then string
	return strValue;
}
