#ifndef QUAXMLSERIALIZER_H
#define QUAXMLSERIALIZER_H

#include <QUaServer>
#include <QDomDocument>
#include <QDomElement>

class QUaXmlSerializer
{
public:
    QUaXmlSerializer();

	void reset();

	QByteArray toByteArray() const;
	bool fromByteArray(const QByteArray &xmlData, QString &strError);

	bool writeInstance(
		const QString& nodeId,
		const QString& typeName,
		const QMap<QString, QVariant>& attrs,
		const QList<QUaForwardReference>& forwardRefs
	);

	bool readInstance(
		const QString& nodeId,
		QString& typeName,
		QMap<QString, QVariant>& attrs,
		QList<QUaForwardReference>& forwardRefs
	);

private:
	// used to hold serialization data
	QDomDocument m_doc;
	// used to hold deserialization data
	struct NodeData 
	{
		QString typeName;
		QMap<QString, QVariant> attrs;
		QList<QUaForwardReference> forwardRefs;
	};
	QMap<QString, NodeData> m_mapNodeData;
	// helper to encode data for serialization
	void writeAttribute(
		QDomElement& node, 
		const QString& strName, 
		const QVariant& varValue
	);
	// helper to decode nodeId
	QString readNodeIdAttribute(
		QDomElement& node,
		QString& strError
	);
};

#endif // QUAXMLSERIALIZER_H
