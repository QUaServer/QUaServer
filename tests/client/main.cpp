// NOTE : needs to come first to avoid winsock redefinition errors
#include "open62541.h"

#include <QCoreApplication>
#include <QDebug>

#include <QConsoleListener>

/*
static UA_INLINE UA_StatusCode
UA_Client_addObjectNode(UA_Client                *client, 
						const UA_NodeId           requestedNewNodeId,
						const UA_NodeId           parentNodeId,
						const UA_NodeId           referenceTypeId,
                        const UA_QualifiedName    browseName,
                        const UA_NodeId           typeDefinition,
                        const UA_ObjectAttributes attr, 
						UA_NodeId                 *outNewNodeId)
*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	// instantiate client objects
	UA_Boolean running = false;
	UA_Client *client  = nullptr;

	auto funcReadTime = [&client, &running]() mutable {
		// early exit
		if (!running)
		{
			qDebug() << "Client not running";
			return;
		}	
		/* Read the value attribute of the node. UA_Client_readValueAttribute is a
		* wrapper for the raw read service available as UA_Client_Service_read. */
		UA_Variant value; /* Variants can hold scalar values and arrays of any type */
		UA_Variant_init(&value);

		/* NodeId of the variable holding the current time */
		const UA_NodeId nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME);
		UA_StatusCode retval = UA_Client_readValueAttribute(client, nodeId, &value);
		// check result
		if(retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME])) 
		{
			UA_DateTime raw_date = *(UA_DateTime *) value.data;
			UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
			qDebug() << QString("Date is: %1-%2-%3 %4:%5:%6.%7").arg(dts.day)
			                                                    .arg(dts.month)
			                                                    .arg(dts.year)
			                                                    .arg(dts.hour)
			                                                    .arg(dts.min)
			                                                    .arg(dts.sec)
			                                                    .arg(dts.milliSec);
		}
		else
		{
			QString strRetCode = UA_StatusCode_name(retval);
			qDebug() << "Failed to read value with code " << strRetCode;
		}
		/* Clean up */
		UA_Variant_deleteMembers(&value);
	};

	auto funcAddPump = [&client, &running]() mutable {
		// early exit
		if (!running)
		{
			qDebug() << "Client not running";
			return;
		}
		// predefined node id of pump object type
		UA_NodeId pumpTypeId = { 1, UA_NODEIDTYPE_NUMERIC,{ 1001 } };
		// create new pump name
		static int npump    = 1;
		QString    strPump  = QString("pump%1").arg(++npump);
		QByteArray bytePump = strPump.toUtf8(); // NOTE : QByteArray must exist in stack, cannot do QByteArray.toUtf8().data();
		char * name = bytePump.data();
		// request add object to server
		UA_NodeId *outNewNodeId;
		UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		oAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", name);
		UA_StatusCode retval = UA_Client_addObjectNode(client,
						                               UA_NODEID_NULL,                              // const UA_NodeId           requestedNewNodeId,
						                               UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),// const UA_NodeId           parentNodeId,
						                               UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),    // const UA_NodeId           referenceTypeId,
                                                       UA_QUALIFIEDNAME(1, name),                   // const UA_QualifiedName    browseName,
                                                       pumpTypeId,                                  // const UA_NodeId           typeDefinition,
                                                       oAttr,                                       // const UA_ObjectAttributes attr, 
						                               outNewNodeId);                               // UA_NodeId                 *outNewNodeId
		// check result
		if (retval == UA_STATUSCODE_GOOD)
		{
			// TODO : UA_NodeId_hash(outNewNodeId) not working, because outNewNodeId == NULL?
			qDebug() << "Sucessfully added pump";
		}
		else
		{
			QString strRetCode = UA_StatusCode_name(retval);
			qDebug() << "Failed to add pump with code " << strRetCode;
		}
	};

	auto funcStopClient = [&client, &running]() mutable {
		// early exit
		if (!running)
		{
			qDebug() << "Client already stopped";
			return;
		}	
		// cleanup and disconnect client
		UA_Client_delete(client);
        running = false;
        qDebug() << "Client stopped";
	};

	// create method to start client from main thread
	auto funcStartClient = [&client, &running, &funcStopClient]() mutable {
		// early exit
		if (running)
		{
			qDebug() << "Client already running";
			return;
		}				
		// configure client
		client = UA_Client_new(UA_ClientConfig_default);   
		// run client (non-blocking)
		UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840");
		if(retval != UA_STATUSCODE_GOOD) 
		{
			QString strRetCode = UA_StatusCode_name(retval);
			qDebug() << "Error starting client with code " << strRetCode;
			// cleanup
	        funcStopClient();
	        return;
	    }
	    // set client running state		
		running = true;
    	qDebug() << "Client started successfully";
	};
	// start client initially
	funcStartClient();

	// listen to console input
	QConsoleListener console;
	QObject::connect(&console, &QConsoleListener::newLine, [&a, 
		                                                    &funcStartClient, 
		                                                    &funcStopClient, 
		                                                    &funcReadTime,
		                                                    &funcAddPump
	                                                       ](const QString &strNewLine) {
		qDebug() << "Echo :" << strNewLine;
		
		// start client
		if (strNewLine.compare("s", Qt::CaseInsensitive) == 0)
		{
			// try to start client
			funcStartClient();
		}
		// read time
		if (strNewLine.compare("t", Qt::CaseInsensitive) == 0)
		{
			// try to read value
			funcReadTime();
		}
		// read value
		if (strNewLine.compare("a", Qt::CaseInsensitive) == 0)
		{
			// try to add pump
			funcAddPump();
		}
		// kill client
		if (strNewLine.compare("x", Qt::CaseInsensitive) == 0)
		{
			qDebug() << "Shutting down client";
			// shutdown client
			funcStopClient();
		}
		// quit app
		if (strNewLine.compare("q", Qt::CaseInsensitive) == 0)
		{
			qDebug() << "Exiting app";
			// shutdown client
			funcStopClient();
			// exit qt app
			a.quit();
		}
	});
	qDebug() << "Listening to console input";
	
    return a.exec();
}
