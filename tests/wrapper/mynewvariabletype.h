#ifndef MYNEWVARIABLETYPE_H
#define MYNEWVARIABLETYPE_H

#include <QOpcUaBaseDataVariable>
#include <QOpcUaBaseObject> // NOTE : if not included, meta-info does not work

class MyOtherNewVariableType;
class MyNewVariableSubType;

class MyNewVariableType : public QOpcUaBaseDataVariable
{
    Q_OBJECT

	Q_PROPERTY(QOpcUaBaseDataVariable * myVar      READ myVar     )
	Q_PROPERTY(QOpcUaBaseObject       * myObj      READ myObj     )
	Q_PROPERTY(MyOtherNewVariableType * myOtherVar READ myOtherVar)
	Q_PROPERTY(MyOtherNewVariableType * myOtherTwo READ myOtherTwo)

public:
	Q_INVOKABLE explicit MyNewVariableType(QOpcUaServer *server);

	QOpcUaBaseDataVariable * myVar();
	QOpcUaBaseObject       * myObj();
	MyOtherNewVariableType * myOtherVar();
	MyOtherNewVariableType * myOtherTwo();

};

// ---

class MyOtherNewVariableType : public QOpcUaBaseDataVariable
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseDataVariable * myVarTwo READ myVarTwo)
public:
	Q_INVOKABLE explicit MyOtherNewVariableType(QOpcUaServer *server);

	QOpcUaBaseDataVariable * myVarTwo();

private:

};

// ---

class MyNewVariableSubType : public MyNewVariableType
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSub READ myObjSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubType(QOpcUaServer *server);

	QOpcUaBaseObject * myObjSub();


};

// ---

class MyNewVariableSubSubType : public MyNewVariableSubType
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSubSub READ myObjSubSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubSubType(QOpcUaServer *server);

	QOpcUaBaseObject * myObjSubSub();


};


#endif // MYNEWVARIABLETYPE_H

