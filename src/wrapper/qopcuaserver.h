#ifndef QOPCUASERVER_H
#define QOPCUASERVER_H

#include <QObject>
#include "open62541.h"

class QOpcUaFolderObject;

class QOpcUaServer : public QObject
{
	Q_OBJECT

friend class QOpcUaServerNode;

public:
    explicit QOpcUaServer(QObject *parent = 0);

	void start();

	QOpcUaFolderObject * get_objectsFolder();
	

signals:

public slots:

private:
	UA_Server * m_server;

	QOpcUaFolderObject * m_pobjectsFolder;
};

#endif // QOPCUASERVER_H