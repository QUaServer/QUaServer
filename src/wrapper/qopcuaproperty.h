#ifndef QOPCUAPROPERTY_H
#define QOPCUAPROPERTY_H

#include <QOpcUaBaseVariable>

// Part 5 - 7.3 : PropertyType
/*
The PropertyType is a subtype of the BaseVariableType. 
It is used as the type definition for all Properties. 
Properties are defined by their BrowseName and therefore they do not need 
a specialised type definition. It is not allowed to subtype this VariableType.
*/

class QOpcUaProperty : public QOpcUaBaseVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QOpcUaProperty(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject);

	// delete methods from derived class
	// https://stackoverflow.com/questions/24609872/delete-virtual-function-from-a-derived-class
	template<typename T = bool>
	QOpcUaProperty * addProperty(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QOpcUaFail<T>::value, "QOpcUaProperty::addProperty, properties cannot have children");
		return nullptr;
	}
	template<typename T = bool>
	QOpcUaBaseDataVariable * addBaseDataVariable(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QOpcUaFail<T>::value, "QOpcUaProperty::addBaseDataVariable, properties cannot have children");
		return nullptr;
	}
	template<typename T = bool>
	QOpcUaBaseObject * addBaseObject(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QOpcUaFail<T>::value, "QOpcUaProperty::addBaseObject, properties cannot have children");
		return nullptr;
	}
	template<typename T = bool>
	QOpcUaFolderObject * addFolderObject(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QOpcUaFail<T>::value, "QOpcUaProperty::addFolderObject, properties cannot have children");
		return nullptr;
	}


};


#endif // QOPCUAPROPERTY_H