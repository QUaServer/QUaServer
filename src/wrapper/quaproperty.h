#ifndef QUAPROPERTY_H
#define QUAPROPERTY_H

#include <QUaBaseVariable>

// Part 5 - 7.3 : PropertyType
/*
The PropertyType is a subtype of the BaseVariableType. 
It is used as the type definition for all Properties. 
Properties are defined by their BrowseName and therefore they do not need 
a specialised type definition. It is not allowed to subtype this VariableType.
*/

class QUaProperty : public QUaBaseVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaProperty(QUaServer *server);

	// delete methods from derived class
	// https://stackoverflow.com/questions/24609872/delete-virtual-function-from-a-derived-class
	template<typename T = bool>
	QUaProperty * addProperty(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QUaFail<T>::value, "QUaProperty::addProperty, properties cannot have children");
		return nullptr;
	}
	template<typename T = bool>
	QUaBaseDataVariable * addBaseDataVariable(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QUaFail<T>::value, "QUaProperty::addBaseDataVariable, properties cannot have children");
		return nullptr;
	}
	template<typename T = bool>
	QUaBaseObject * addBaseObject(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QUaFail<T>::value, "QUaProperty::addBaseObject, properties cannot have children");
		return nullptr;
	}
	template<typename T = bool>
	QUaFolderObject * addFolderObject(const QString &strBrowseName = "")
	{
		Q_UNUSED(strBrowseName);
		Q_STATIC_ASSERT_X(QUaFail<T>::value, "QUaProperty::addFolderObject, properties cannot have children");
		return nullptr;
	}


};


#endif // QUAPROPERTY_H