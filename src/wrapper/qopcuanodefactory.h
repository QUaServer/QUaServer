#ifndef QOPCUANODEFACTORY_H
#define QOPCUANODEFACTORY_H

#include <QOpcUaServerNode>

// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern#Static_polymorphism

template <class T>
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

template <class T> 
UA_NodeId QOpcUaNodeFactory<T>::m_typeNodeId = UA_NODEID_NULL;

#endif // QOPCUANODEFACTORY_H
