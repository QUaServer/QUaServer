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
	bool fromByteArray(
		const QUaServer* server,
		const QByteArray &xmlData, 
		QString &strError
	);

	bool writeInstance(
		const QString &nodeId,
		const QString &typeName,
		const QMap<QString, QVariant> &attrs,
		const QList<QUaForwardReference> &forwardRefs,
		QQueue<QUaLog> &logOut
	);

	bool readInstance(
		const QString &nodeId,
		QString &typeName,
		QMap<QString, QVariant> &attrs,
		QList<QUaForwardReference> &forwardRefs,
		QQueue<QUaLog> &logOut
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
	// helper to decode typeName
	QString readTypeNameAttribute(
		const QUaServer * server,
		QDomElement& node,
		QString& strError
	);
	// helper to decode serialized data
	QVariant readAttribute(
		const QString& strValue,
		QString& strError
	);
};

#endif // QUAXMLSERIALIZER_H
