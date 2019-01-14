#ifndef QOPCUASERVERNODE_H
#define QOPCUASERVERNODE_H

#include <QOpcUaServer>

/*
typedef struct {
	// Node Attributes
	UA_UInt32        specifiedAttributes;
	UA_LocalizedText displayName;
	UA_LocalizedText description;
	UA_UInt32        writeMask;
	UA_UInt32        userWriteMask;
} UA_NodeAttributes;
*/

class QOpcUaServerNode : public QObject
{
    Q_OBJECT

	// Node Attributes

	// N/A
	//Q_PROPERTY(quint32 specifiedAttributes READ get_specifiedAttributes)

	Q_PROPERTY(QString displayName READ get_displayName WRITE set_displayName)
	Q_PROPERTY(QString description READ get_description WRITE set_description)
	Q_PROPERTY(quint32 writeMask   READ get_writeMask   WRITE set_writeMask  )

	// Cannot be read, since the local "admin" user always has full rights.
	// Cannot be written from the server, as they are specific to the different users and set by the access control callback.
	//Q_PROPERTY(quint32 userWriteMask READ get_userWriteMask)

	// Node Specifics

	// Cannot be changed once a node has been created.
	Q_PROPERTY(QString nodeId     READ get_nodeId   )
	Q_PROPERTY(QString nodeClass  READ get_nodeClass)

	// Other

	Q_PROPERTY(QPair<quint16, QString> browseName READ get_browseName WRITE set_browseName)

public:
	explicit QOpcUaServerNode(QOpcUaServerNode *parent);

	QString get_displayName() const;
	void    set_displayName(const QString &displayName);
	QString get_description() const;
	void    set_description(const QString &description);
	quint32 get_writeMask  () const;
	void    set_writeMask  (const quint32 &writeMask);

	QString get_nodeId     () const;
	QString get_nodeClass  () const;
	
	QPair<quint16, QString> get_browseName() const;
	void                    set_browseName(const QPair<quint16, QString> &browseName);

	// private?

	// to be able to reuse methods in subclasses
	QOpcUaServer * m_qopcuaserver;

	// protected?

	// INSTANCE NodeId
	UA_NodeId m_nodeId;

	static UA_NodeId nodeIdFromQString  (const QString &name);
	static QString   nodeIdToQString    (const UA_NodeId &id);
	static bool      nodeIdStringSplit  (const QString &nodeIdString, quint16 *nsIndex, QString *identifier, char *identifierType);
	static QString   nodeClassToQString (const UA_NodeClass &nclass);
	static QString   uaStringToQString  (const UA_String &string);
	static UA_String uaStringFromQString(const QString &uaString);

protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaServerNode(QOpcUaServer *server);

private:

};

#endif // QOPCUASERVERNODE_H