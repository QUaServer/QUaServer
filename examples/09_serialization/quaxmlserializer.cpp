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
	// close file
	m_xmlFileConf.close();
}

QByteArray QUaXmlSerializer::toByteArray() const
{
	return m_doc.toByteArray();
}

bool QUaXmlSerializer::fromByteArray(
	const QByteArray& xmlData, 
	QQueue<QUaLog>& logOut)
{
	// load from xml
	int line, col;
	QString strXmlError;
	m_doc.setContent(xmlData, &strXmlError, &line, &col);
	if (!strXmlError.isEmpty())
	{
		logOut << QUaLog({
			QObject::tr("Invalid XML in Line %1 Column %2 Error %3")
				.arg(line)
				.arg(col)
				.arg(strXmlError),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// fill up nodes data
	QDomElement root = m_doc.documentElement();
	QDomNode nIter = root.firstChild();
	while (!nIter.isNull()) 
	{
		// try to convert the node to an element, continue if not possible
		QDomElement node = nIter.toElement(); 
		if (node.isNull()) 
		{
			nIter = nIter.nextSibling();
			continue;
		}
		// parse nodeId
		QString nodeId = this->readNodeIdAttribute(node, logOut);
		if (nodeId.isEmpty())
		{
			nIter = nIter.nextSibling();
			continue;
		}
		// read rest of attributes
		QMap<QString, QVariant> mapAttrs;
		auto attrs = node.attributes();
		for (int i = 0; i < attrs.count(); i++)
		{
			QDomAttr attr = attrs.item(i).toAttr();
			if (attr.isNull() || 
				attr.name().compare("nodeId") == 0)
			{
				continue;
			}
			// deserialize
			QString name = attr.name();
			QVariant value = this->readAttribute(attr.value(), logOut);
			if (!value.isValid())
			{
				continue;
			}
			Q_ASSERT(!mapAttrs.contains(name));
			mapAttrs.insert(name, value);
		}
		// parse references
		QList<QUaForwardReference> refs;
		QDomNode rIter = node.firstChild();
		while (!rIter.isNull())
		{
			// try to convert the node to an element, continue if not possible
			QDomElement ref = rIter.toElement();
			if (ref.isNull())
			{
				rIter = rIter.nextSibling();
				continue;
			}
			// parse targetNodeId
			QString targetNodeId = this->readNodeIdTargetAttribute(ref, logOut);
			if (targetNodeId.isEmpty())
			{
				rIter = rIter.nextSibling();
				continue;
			}
			// parse targetType
			QString targetType = this->readTargetTypeAttribute(ref, logOut);
			if (targetType.isEmpty())
			{
				rIter = rIter.nextSibling();
				continue;
			}
			QUaReferenceType refType = this->readRefNameAttribute(ref, logOut);
			if (refType.strForwardName.isEmpty() || refType.strInverseName.isEmpty())
			{
				rIter = rIter.nextSibling();
				continue;
			}
			// success parsing fRef
			refs << QUaForwardReference({
				targetNodeId,
				targetType,
				refType
			});
			// continue with next element
			rIter = rIter.nextSibling();
		}
		// insert to internal data
		m_mapNodeData.insert(nodeId, {
			mapAttrs,
			refs
		});
		// continue with next element
		nIter = nIter.nextSibling();
	}
	return true;
}

QString QUaXmlSerializer::xmlFileName() const
{
	return m_strXmlFileName;
}

bool QUaXmlSerializer::setXmlFileName(
	const QString& strXmlFileName, 
	QQueue<QUaLog>& logOut)
{
	Q_UNUSED(logOut);
	// copy internally
	m_strXmlFileName = strXmlFileName;
	// reset internal state (close file, etc.)
	this->reset();
	// set filename
	m_xmlFileConf.setFileName(m_strXmlFileName);
	// always success
	return true;
}

bool QUaXmlSerializer::serializeStart(QQueue<QUaLog>& logOut)
{
	// reset internal state
	this->reset();
	// if we cannot open file, then no point in continuing with serialization
	if (!m_xmlFileConf.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate))
	{
		logOut << QUaLog({
			QObject::tr("Could not open file %1.").arg(m_strXmlFileName),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// keep file open
	return true;
}

bool QUaXmlSerializer::serializeEnd(QQueue<QUaLog>& logOut)
{
	if (!m_xmlFileConf.isOpen())
	{
		logOut << QUaLog({
			QObject::tr("File %1 is not open.").arg(m_strXmlFileName),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return false;
	}
	// create stream
	QTextStream streamConfig(&m_xmlFileConf);
	// save config in file
	auto b = this->toByteArray();
	streamConfig << b;
	// close file
	m_xmlFileConf.close();
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
	Q_UNUSED(typeName);
	QDomElement root = m_doc.documentElement();
	// create node in xml
	QDomElement node = m_doc.createElement("n");
	root.appendChild(node);
	// copy attributes
	this->writeAttribute(node, "nodeId", nodeId);
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
		this->writeAttribute(refElem, "targetNodeId", ref.targetNodeId);
		this->writeAttribute(refElem, "targetType"  , ref.targetType);
		this->writeAttribute(refElem, "forwardName" , ref.refType.strForwardName);
		this->writeAttribute(refElem, "inverseName" , ref.refType.strInverseName);
	}
	return true;
}

bool QUaXmlSerializer::deserializeStart(QQueue<QUaLog>& logOut)
{
	// reset internal state
	this->reset();
	// if we cannot open file, then no point in continuing with deserialization
	if (!m_xmlFileConf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		logOut << QUaLog({
			QObject::tr("Could not open file %1.").arg(m_strXmlFileName),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
			});
		return false;
	}
	// load all data
	if (!this->fromByteArray(m_xmlFileConf.readAll(), logOut))
	{
		// print log entries if any
		for (auto log : logOut)
		{
			qWarning() << "[" << log.level << "] :" << log.message;
		}
		// close file
		m_xmlFileConf.close();
		// exit
		return false;
	}
	// close file
	m_xmlFileConf.close();
	// exit
	return true;
}

bool QUaXmlSerializer::deserializeEnd(QQueue<QUaLog>& logOut)
{
	Q_UNUSED(logOut);
	// cleanup
	this->reset();
	return true;
}

bool QUaXmlSerializer::readInstance(
	const QString& nodeId, 
	const QString& typeName, 
	QMap<QString, QVariant>& attrs, 
	QList<QUaForwardReference>& forwardRefs,
	QQueue<QUaLog>& logOut)
{
	Q_UNUSED(typeName);
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

QString QUaXmlSerializer::readNodeIdAttribute(
	QDomElement& node, 
	QQueue<QUaLog>& logOut)
{
	if (!node.hasAttribute("nodeId"))
	{
		logOut << QUaLog({
			QObject::tr("Found node element without nodeId attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return "";
	}
	QString nodeId = node.attribute("nodeId", "");
	if (nodeId.isEmpty())
	{
		logOut << QUaLog({
			QObject::tr("Found node element with empty nodeId attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return "";
	}
	if (!QUaServer::isIdValid(nodeId))
	{
		logOut << QUaLog({
			QObject::tr("Found node element with invalid nodeId attribute %1. Ignoring.").arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return "";
	}
	// success
	return nodeId;
}

QVariant QUaXmlSerializer::readAttribute(
	const QString& strValue, 
	QQueue<QUaLog>& logOut)
{
	Q_UNUSED(logOut);
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

QString QUaXmlSerializer::readNodeIdTargetAttribute(
	QDomElement& ref, 
	QQueue<QUaLog>& logOut)
{
	if (!ref.hasAttribute("targetNodeId"))
	{
		logOut << QUaLog({
			QObject::tr("Found reference element without targetNodeId attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return "";
	}
	QString targetNodeId = ref.attribute("targetNodeId", "");
	if (targetNodeId.isEmpty())
	{
		logOut << QUaLog({
			QObject::tr("Found reference element with empty targetNodeId attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
	}
	return targetNodeId;
}

QString QUaXmlSerializer::readTargetTypeAttribute(
	QDomElement& ref, 
	QQueue<QUaLog>& logOut)
{
	if (!ref.hasAttribute("targetType"))
	{
		logOut << QUaLog({
			QObject::tr("Found reference element without targetType attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return "";
	}
	QString targetType = ref.attribute("targetType", "");
	if (targetType.isEmpty())
	{
		logOut << QUaLog({
			QObject::tr("Found reference element with empty targetType attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return "";
	}
	return targetType;
}

QUaReferenceType QUaXmlSerializer::readRefNameAttribute(
	QDomElement& ref, 
	QQueue<QUaLog>& logOut)
{
	if (!ref.hasAttribute("forwardName"))
	{
		logOut << QUaLog({
			QObject::tr("Found reference element without forwardName attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return {"", ""};
	}
	QString forwardName = ref.attribute("forwardName", "");
	if (forwardName.isEmpty())
	{
		logOut << QUaLog({
			QObject::tr("Found reference element with empty forwardName attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return { "", "" };
	}
	if (!ref.hasAttribute("inverseName"))
	{
		logOut << QUaLog({
			QObject::tr("Found reference element without inverseName attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return { "", "" };
	}
	QString inverseName = ref.attribute("inverseName", "");
	if (inverseName.isEmpty())
	{
		logOut << QUaLog({
			QObject::tr("Found reference element with empty inverseName attribute. Ignoring."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		});
		return { "", "" };
	}
	QUaReferenceType refType = {
		forwardName,
		inverseName
	};
	return refType;
}
