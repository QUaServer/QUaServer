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
