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

	static UA_NodeId m_typeNodeId;
};

template <typename T>
UA_NodeId QOpcUaNodeFactory<T>::m_typeNodeId = UA_NODEID_NULL;

/*
template <typename T>
struct QOpcUaObjectFactory
{
public:


};


*/

template <typename T>
struct QOpcUaVariableFactory
{
	static QVariant GetValue()
	{
		return QVariant((QVariant::Type)QMetaType::UnknownType);
	}

	static quint8   GetAccessLevel()
	{
		return UA_ACCESSLEVELMASK_READ;
	}

};

#endif // QOPCUANODEFACTORY_H
