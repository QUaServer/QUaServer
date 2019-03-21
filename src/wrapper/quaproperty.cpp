#include "quaproperty.h"

#include <QUaServer>
#include <QUaBaseDataVariable>
#include <QUaFolderObject>

QOpcUaProperty::QOpcUaProperty(QOpcUaServer *server)
	: QOpcUaBaseVariable(server)
{

}