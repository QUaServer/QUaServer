#ifndef QOPCUANODEFACTORY_H
#define QOPCUANODEFACTORY_H

#include <QOpcUaServerNode>

// [TRAITS] : default implementation
template <typename T>
struct QOpcUaNodeFactory
{
    static UA_NodeId GetTypeNodeId()
    {
        return m_typeNodeId;
    }

    static void SetTypeNodeId(const UA_NodeId & typeNodeId)
    {
		m_typeNodeId = typeNodeId;
    }

	static UA_NodeId m_typeNodeId;
};

template <typename T>
UA_NodeId QOpcUaNodeFactory<T>::m_typeNodeId = UA_NODEID_NULL;

template <typename T>
struct QOpcUaObjectTypeTraits
{
	static QString GetBrowseName()
	{
		return QString();
	}

	// node attributes

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

	// object type attributes

	static bool GetIsAbstract()
	{
		return false;
	}
};

template <typename T>
struct QOpcUaVariableTypeTraits
{
	static QString GetBrowseName()
	{
		return QString();
	}

	// node attributes

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

	// variable type attributes

	static QVariant GetDefaultValue()
	{
		return QVariant((QVariant::Type)QMetaType::UnknownType);
	}

	static bool GetIsAbstract()
	{
		return false;
	}

};

#endif // QOPCUANODEFACTORY_H
