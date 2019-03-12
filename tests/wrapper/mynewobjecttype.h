#ifndef MYNEWOBJECTTYPE_H
#define MYNEWOBJECTTYPE_H

#include <QOpcUaBaseObject>
#include "mynewvariabletype.h"

class MyNewObjectType : public QOpcUaBaseObject
{
    Q_OBJECT

	Q_PROPERTY(QOpcUaBaseDataVariable  * myVar       READ myVar      )
	Q_PROPERTY(MyNewVariableSubSubType * myVarSubSub READ myVarSubSub)

public:
	Q_INVOKABLE explicit MyNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject);

	MyNewVariableSubSubType * myVarSubSub();
	QOpcUaBaseDataVariable  * myVar();


private:
	
};

class MyOtherNewObjectType : public QOpcUaBaseObject
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit MyOtherNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject);

private:


};


#endif // MYNEWOBJECTTYPE_H

