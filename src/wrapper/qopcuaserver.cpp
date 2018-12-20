#include "qopcuaserver.h"

#include <QOpcUaFolderObject>

/*
UA_EXPORT UA_ServerConfig *
UA_ServerConfig_new_minimal(UA_UInt16 portNumber, const UA_ByteString *certificate);
*/

QOpcUaServer::QOpcUaServer(QObject *parent) : QObject(parent)
{
	UA_ServerConfig *config = UA_ServerConfig_new_default();
	m_server = UA_Server_new(config);
	m_pobjectsFolder = new QOpcUaFolderObject(this);
}

void QOpcUaServer::start()
{
	// NOTE : seems e must define port and other server params upon instantiation, 
	//        because rest of API assumes m_server is valid
	UA_Boolean running = true;
	UA_Server_run(m_server, &running);
}

QOpcUaFolderObject * QOpcUaServer::get_objectsFolder()
{
	return m_pobjectsFolder;
}
