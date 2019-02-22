#include "qopcuaproperty.h"

#include <QOpcUaServer>
#include <QOpcUaBaseDataVariable>
#include <QOpcUaFolderObject>

QOpcUaProperty::QOpcUaProperty(QOpcUaServerNode *parent) : QOpcUaBaseVariable(parent)
{

}

//template<typename T>
//QOpcUaProperty * QOpcUaProperty::addProperty(const QString &strBrowseName/* = ""*/)
//{
//	Q_UNUSED(strBrowseName);
//	static_assert (fail<T>::value, "Do not use!");
//	return nullptr;
//}
//
//QOpcUaBaseDataVariable * QOpcUaProperty::addBaseDataVariable(const QString &strBrowseName/* = ""*/)
//{
//	Q_UNUSED(strBrowseName);
//	return nullptr;
//}
//
//QOpcUaBaseObject * QOpcUaProperty::addBaseObject(const QString &strBrowseName/* = ""*/)
//{
//	Q_UNUSED(strBrowseName);
//	return nullptr;
//}
//
//QOpcUaFolderObject * QOpcUaProperty::addFolderObject(const QString &strBrowseName/* = ""*/)
//{
//	Q_UNUSED(strBrowseName);
//	return nullptr;
//}