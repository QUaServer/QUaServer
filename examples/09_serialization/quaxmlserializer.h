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
	QDomDocument m_doc;
	void writeAttribute(
		QDomElement& node, 
		const QString& strName, 
		const QVariant& varValue
	);
};

#endif // QUAXMLSERIALIZER_H
