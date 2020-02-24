#ifndef QUAXMLSERIALIZER_H
#define QUAXMLSERIALIZER_H

#include <QUaServer>
#include <QDomDocument>
#include <QDomElement>

class QUaXmlSerializer
{
public:
    QUaXmlSerializer();

	// reset serializer state
	void reset();

	// write text from XML
	QByteArray toByteArray() const;

	// read XML from text
	bool fromByteArray(
		const QUaServer* server,
		const QByteArray &xmlData, 
		QQueue<QUaLog>& logOut
	);

	// required API for QUaNode::serialize
	bool writeInstance(
		const QString &nodeId,
		const QString &typeName,
		const QMap<QString, QVariant> &attrs,
		const QList<QUaForwardReference> &forwardRefs,
		QQueue<QUaLog> &logOut
	);

	// required API for QUaNode::deserialize
	bool readInstance(
		const QString &nodeId,
		QString &typeName,
		QMap<QString, QVariant> &attrs,
		QList<QUaForwardReference> &forwardRefs,
		QQueue<QUaLog> &logOut
	);

private:
	// used to hold serialization state
	QDomDocument m_doc;
	// used to hold deserialization state
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
		QQueue<QUaLog>& logOut
	);
	// helper to decode typeName
	QString readTypeNameAttribute(
		const QUaServer * server,
		QDomElement& node,
		QQueue<QUaLog>& logOut
	);
	// helper to decode serialized data
	QVariant readAttribute(
		const QString& strValue,
		QQueue<QUaLog>& logOut
	);
	// helper to decode targetNodeId
	QString readNodeIdTargetAttribute(
		QDomElement& ref,
		QQueue<QUaLog>& logOut
	);
	// helper to decode targetType
	QString readTargetTypeAttribute(
		const QUaServer* server,
		QDomElement& ref,
		QQueue<QUaLog>& logOut
	);
	// helper to decode forwardName and inverseName
	QUaReferenceType readRefNameAttribute(
		const QUaServer* server,
		QDomElement& ref,
		QQueue<QUaLog>& logOut
	);
};

#endif // QUAXMLSERIALIZER_H
