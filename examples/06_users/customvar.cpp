#include "customvar.h"

CustomVar::CustomVar(QUaServer *server)
	: QUaBaseDataVariable(server)
{
	this->myProp()->setValue("xxx");
	this->varFoo()->setValue(true);
	this->varBar()->setValue(69);
	this->myProp()->setWriteAccess(true);
	this->varFoo()->setWriteAccess(true);
	this->varBar()->setWriteAccess(true);
}

QUaProperty * CustomVar::myProp()
{
	return this->browseChild<QUaProperty>("myProp");	
}

QUaBaseDataVariable * CustomVar::varFoo()
{
	return this->browseChild<QUaBaseDataVariable>("varFoo");
}

QUaBaseDataVariable * CustomVar::varBar()
{
	return this->browseChild<QUaBaseDataVariable>("varBar");
}

QUaAccessLevel CustomVar::userAccessLevel(const QString & strUserName)
{
	QUaAccessLevel access;
	// Read Access to all
	access.bits.bRead = true;
	// Write Access only to john
	if (strUserName.compare("john", Qt::CaseSensitive) == 0)
	{
		access.bits.bWrite = true;
	}
	else
	{
		access.bits.bWrite = false;
	}
	return access;
}
