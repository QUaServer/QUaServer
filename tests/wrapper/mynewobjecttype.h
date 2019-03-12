#ifndef MYNEWOBJECTTYPE_H
#define MYNEWOBJECTTYPE_H

#include <QOpcUaBaseObject>
#include <QOpcUaProperty>
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

	Q_INVOKABLE bool    updateMyVar(quint32 newVarVal);
	Q_INVOKABLE QString saluteName (QString strName);
	Q_INVOKABLE double  divideNums (int intNum, int intDen);

private:
	
};

class MyOtherNewObjectType : public QOpcUaBaseObject
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit MyOtherNewObjectType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject);

private:


};

class MyNewObjectSubType : public MyNewObjectType
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaProperty * myProp READ myProp)

public:
	Q_INVOKABLE explicit MyNewObjectSubType(QOpcUaServer *server, const UA_NodeId &nodeId, const QMetaObject & metaObject);

	QOpcUaProperty * myProp();

	Q_INVOKABLE QString concatArgs(int intNum, double dblNum, QString strName);

private:


};


#endif // MYNEWOBJECTTYPE_H

