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

/*
template <class T>
class QOpcUaObjectFactory : public QOpcUaNodeFactory<T>
{
public:


};
*/

#endif // QOPCUANODEFACTORY_H
