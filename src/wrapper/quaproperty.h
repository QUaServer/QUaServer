#ifndef QUAPROPERTY_H
#define QUAPROPERTY_H

#include <QUaBaseVariable>

class QUaProperty : public QUaBaseVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaProperty(QUaServer *server);

	// NOTE : use of QUaFail<T>::value to delete methods from derived class
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