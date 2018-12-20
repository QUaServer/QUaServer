#ifndef QOPCUAFOLDEROBJECT_H
#define QOPCUAFOLDEROBJECT_H

#include <QOpcUaBaseObject>

class QOpcUaFolderObject : public QOpcUaBaseObject
{
    Q_OBJECT

friend class QOpcUaServer;

public:
    explicit QOpcUaFolderObject(QOpcUaServerNode *parent);

	QOpcUaBaseObject * addBaseObject(const QString &displayName = "", const QString &description = "");

	// TODO : addBaseDataVariable

private:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaFolderObject(QOpcUaServer *server);
};

#endif // QOPCUAFOLDEROBJECT_H