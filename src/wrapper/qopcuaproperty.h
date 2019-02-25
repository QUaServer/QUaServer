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
    explicit QOpcUaProperty(QOpcUaServerNode *parent);

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

// [TRAITS] : specialization
template <>
struct QOpcUaNodeFactory<QOpcUaProperty>
{
	static UA_NodeId GetTypeNodeId()
	{
		return UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE);
	}

	static void SetTypeNodeId(const UA_NodeId & typeNodeId)
	{
		Q_UNUSED(typeNodeId);
	}

	static QString GetDisplayName()
	{
		return QString();
	}

	static QString GetDescription()
	{
		return QString();
	}

	static quint32 GetWriteMask()
	{
		return 0;
	}
};


#endif // QOPCUAPROPERTY_H