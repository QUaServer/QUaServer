#include "quaxmlserializer.h"

QUaXmlSerializer::QUaXmlSerializer()
{
	this->reset();
}

void QUaXmlSerializer::reset()
{
	m_doc.clear();
	// setup
	QDomProcessingInstruction header = m_doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	m_doc.appendChild(header);
	QDomElement root = m_doc.createElement("nodes");
	m_doc.appendChild(root);
}

QByteArray QUaXmlSerializer::toByteArray() const
{
	return m_doc.toByteArray();
}

bool QUaXmlSerializer::fromByteArray(const QByteArray& xmlData, QString& strError)
{
	// load from xml
	QDomDocument doc;
	int line, col;
	doc.setContent(xmlData, &strError, &line, &col);
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
		

		// TODO : parse attribute

		// continue with next element
		iter = iter.nextSibling();
	}
	// reset internal xml doc
	this->reset();
	return true;
}

bool QUaXmlSerializer::writeInstance(
	const QString& nodeId, 
	const QString& typeName, 
	const QMap<QString, QVariant>& attrs, 
	const QList<QUaForwardReference>& forwardRefs)
{
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
		this->writeAttribute(refElem, "forwardName" , ref.refType.strForwardName);
		this->writeAttribute(refElem, "inverseName" , ref.refType.strInverseName);
	}
	return true;
}

bool QUaXmlSerializer::readInstance(
	const QString& nodeId, 
	QString& typeName, 
	QMap<QString, QVariant>& attrs, 
	QList<QUaForwardReference>& forwardRefs)
{
	// TODO
	return false;
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

	// TODO : validate

}
