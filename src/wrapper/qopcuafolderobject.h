#ifndef QOPCUAFOLDEROBJECT_H
#define QOPCUAFOLDEROBJECT_H

#include <QOpcUaBaseObject>
#include <QOpcUaBaseDataVariable>

class QOpcUaFolderObject : public QOpcUaAbstractObject, public QOpcUaNodeFactory<QOpcUaFolderObject>
{
    Q_OBJECT

friend class QOpcUaServer;

public:
    explicit QOpcUaFolderObject(QOpcUaServerNode *parent);

	QOpcUaBaseObject * addBaseObject();

	QOpcUaBaseDataVariable * addBaseDataVariable();

	QOpcUaFolderObject * addFolderObject();

	static UA_NodeId m_typeNodeId;

private:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaFolderObject(QOpcUaServer *server);
};

#endif // QOPCUAFOLDEROBJECT_H