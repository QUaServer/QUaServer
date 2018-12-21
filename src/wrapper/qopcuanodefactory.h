#ifndef QOPCUANODEFACTORY_H
#define QOPCUANODEFACTORY_H

#include <QOpcUaServerNode>

// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern#Static_polymorphism

template <class T>
struct QOpcUaNodeFactory
{
    static UA_NodeId GetTypeNodeId()
    {
        return T::m_typeNodeId;
    }

    static void SetTypeNodeId(const UA_NodeId & typeNodeId)
    {
		T::m_typeNodeId = typeNodeId;
    }

};

#endif // QOPCUANODEFACTORY_H
