#include "qopcuaproperty.h"

#include <QOpcUaServer>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaFolderObject>

QOpcUaProperty::QOpcUaProperty(QOpcUaServer *server)
	: QOpcUaBaseVariable(server)
{

}