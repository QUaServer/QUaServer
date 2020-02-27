#ifndef QUAXMLSERIALIZER_H
#define QUAXMLSERIALIZER_H

#include <QUaServer>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>

class QUaXmlSerializer
{
public:
    QUaXmlSerializer();

	// set XML file to read from or write to
	QString xmlFileName() const;
	bool setXmlFileName(
		const QString& strXmlFileName,
		QQueue<QUaLog>& logOut
	);

	// optional API for QUaNode::serialize
	bool serializeStart(QQueue<QUaLog>& logOut);

	// optional API for QUaNode::serialize
	bool serializeEnd(QQueue<QUaLog>& logOut);

	// required API for QUaNode::serialize
	bool writeInstance(
		const QString &nodeId,
		const QString &typeName,
		const QMap<QString, QVariant> &attrs,
		const QList<QUaForwardReference> &forwardRefs,
		QQueue<QUaLog> &logOut
	);

	// optional API for QUaNode::deserialize
	bool deserializeStart(QQueue<QUaLog>& logOut);

	// optional API for QUaNode::deserialize
	bool deserializeEnd(QQueue<QUaLog>& logOut);

	// required API for QUaNode::deserialize
	bool readInstance(
		const QString &nodeId,
		const QString &typeName,
		QMap<QString, QVariant> &attrs,
		QList<QUaForwardReference> &forwardRefs,
		QQueue<QUaLog> &logOut
	);

private:
	QString m_strXmlFileName;
	QFile   m_xmlFileConf;
	// used to hold serialization state
	QDomDocument m_doc;
	// used to hold deserialization state
	struct NodeData 
	{
		QMap<QString, QVariant> attrs;
		QList<QUaForwardReference> forwardRefs;
	};
	QMap<QString, NodeData> m_mapNodeData;
	// reset serializer state
	void reset();
	// write text from XML
	QByteArray toByteArray() const;
	// read XML from text
	bool fromByteArray(
		const QByteArray& xmlData,
		QQueue<QUaLog>& logOut
	);
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
		QDomElement& ref,
		QQueue<QUaLog>& logOut
	);
	// helper to decode forwardName and inverseName
	QUaReferenceType readRefNameAttribute(
		QDomElement& ref,
		QQueue<QUaLog>& logOut
	);
};

#endif // QUAXMLSERIALIZER_H
