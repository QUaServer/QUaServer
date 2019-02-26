#include "qopcuaserver.h"

QOpcUaServer::QOpcUaServer(QObject *parent) : QObject(parent)
{
	UA_ServerConfig *config  = UA_ServerConfig_new_default();
	this->m_server = UA_Server_new(config);
	m_running = false;
	// Create "Objects" folder using special constructor
	// Part 5 - 8.2.4 : Objects
	m_pobjectsFolder = new QOpcUaFolderObject(this);
	// register base types
	m_mapTypes.insert(QString(QOpcUaBaseVariable    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE)    );
	m_mapTypes.insert(QString(QOpcUaBaseDataVariable::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE));
	m_mapTypes.insert(QString(QOpcUaProperty        ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE)        );
	m_mapTypes.insert(QString(QOpcUaBaseObject      ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)      );
	m_mapTypes.insert(QString(QOpcUaFolderObject    ::staticMetaObject.className()), UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)          );
}

/* TODO : alternative constructor
UA_EXPORT UA_ServerConfig * UA_ServerConfig_new_minimal(UA_UInt16 portNumber, const UA_ByteString *certificate);
*/

void QOpcUaServer::start()
{
	// NOTE : we must define port and other server params upon instantiation, 
	//        because rest of API assumes m_server is valid
	if (m_running)
	{
		return;
	}
	auto st = UA_Server_run_startup(this->m_server);
	Q_ASSERT(st == UA_STATUSCODE_GOOD);
	Q_UNUSED(st)
	m_running = true;
	m_connection = QObject::connect(this, &QOpcUaServer::iterateServer, this, [this]() {
		if (m_running) 
		{
			UA_Server_run_iterate(this->m_server, true);
			// iterate again
			emit this->iterateServer();
		}	
	}, Qt::QueuedConnection);
	// bootstrap iterations
	emit this->iterateServer();
}

void QOpcUaServer::stop()
{
	m_running = false;
	QObject::disconnect(m_connection);
	UA_Server_run_shutdown(this->m_server);
}

bool QOpcUaServer::isRunning()
{
	return m_running;
}

QOpcUaFolderObject * QOpcUaServer::get_objectsFolder()
{
	return m_pobjectsFolder;
}

UA_NodeId QOpcUaServer::getReferenceTypeId(const QString & strParentClassName, const QString & strChildClassName)
{
	UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	// adapt parent relation with child according to parent type
	if (strParentClassName.compare(QOpcUaFolderObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
	{
		referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	}
	else if (strParentClassName.compare(QOpcUaBaseObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
	{
		if (strChildClassName.compare(QOpcUaFolderObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0 ||
			strChildClassName.compare(QOpcUaBaseObject::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT);
		}
		else if (strChildClassName.compare(QOpcUaBaseDataVariable::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
		}
		else if (strChildClassName.compare(QOpcUaProperty::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
		{
			referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
		}
	}
	else if (strParentClassName.compare(QOpcUaBaseDataVariable::staticMetaObject.className(), Qt::CaseInsensitive) == 0)
	{
		// TODO
		referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
	}
	else
	{
		Q_ASSERT_X(false, "QOpcUaServer::getReferenceTypeId", "Invalid parent type.");
	}
	
	return referenceTypeId;
}
