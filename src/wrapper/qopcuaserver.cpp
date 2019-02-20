#include "qopcuaserver.h"

#include <QOpcUaFolderObject>

QOpcUaServer::QOpcUaServer(QObject *parent) : QObject(parent)
{
	UA_ServerConfig *config  = UA_ServerConfig_new_default();
	this->m_server = UA_Server_new(config);
	// Create "Objects" folder using special constructor
	// Part 5 - 8.2.4 : Objects
	m_pobjectsFolder = new QOpcUaFolderObject(this);
}

/* TODO : alternative constructor
UA_EXPORT UA_ServerConfig * UA_ServerConfig_new_minimal(UA_UInt16 portNumber, const UA_ByteString *certificate);
*/

void QOpcUaServer::start()
{
	// NOTE : we must define port and other server params upon instantiation, 
	//        because rest of API assumes m_server is valid
	UA_Boolean running = true;
	UA_Server_run(this->m_server, &running);
}

QOpcUaFolderObject * QOpcUaServer::get_objectsFolder()
{
	return m_pobjectsFolder;
}
