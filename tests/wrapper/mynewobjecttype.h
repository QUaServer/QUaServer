#ifndef MYNEWOBJECTTYPE_H
#define MYNEWOBJECTTYPE_H

#include <QOpcUaBaseObject>
#include "mynewvariabletype.h"

class MyNewObjectType : public QOpcUaBaseObject
{
    Q_OBJECT

	Q_PROPERTY(MyNewVariableSubSubType * myVarSubSub READ getMyVarSubSub)

public:
	Q_INVOKABLE explicit MyNewObjectType(QOpcUaServerNode *parent);

	MyNewVariableSubSubType * getMyVarSubSub();

private:
	MyNewVariableSubSubType * m_myVarSubSub;
	
};

class MyOtherNewObjectType : public QOpcUaBaseObject
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit MyOtherNewObjectType(QOpcUaServerNode *parent);

private:


};


#endif // MYNEWOBJECTTYPE_H

