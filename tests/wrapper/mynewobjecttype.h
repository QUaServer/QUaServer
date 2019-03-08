#ifndef MYNEWOBJECTTYPE_H
#define MYNEWOBJECTTYPE_H

#include <QOpcUaBaseObject>
#include "mynewvariabletype.h"

class MyNewObjectType : public QOpcUaBaseObject, public QOpcUaServerNodeFactory<MyNewObjectType>
{
    Q_OBJECT

	Q_PROPERTY(MyNewVariableSubSubType * myVarSubSub READ getMyVarSubSub)

public:
	Q_INVOKABLE explicit MyNewObjectType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	MyNewVariableSubSubType * getMyVarSubSub();


private:
	
};

class MyOtherNewObjectType : public QOpcUaBaseObject, public QOpcUaServerNodeFactory<MyOtherNewObjectType>
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit MyOtherNewObjectType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

private:


};


#endif // MYNEWOBJECTTYPE_H

