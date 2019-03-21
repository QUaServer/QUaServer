#ifndef MYNEWVARIABLETYPE_H
#define MYNEWVARIABLETYPE_H

#include <QUaBaseDataVariable>
#include <QUaBaseObject> // NOTE : if not included, meta-info does not work

class MyOtherNewVariableType;
class MyNewVariableSubType;

class MyNewVariableType : public QUaBaseDataVariable
{
    Q_OBJECT

	Q_PROPERTY(QUaBaseDataVariable    * myVar      READ myVar     )
	Q_PROPERTY(QUaBaseObject          * myObj      READ myObj     )
	Q_PROPERTY(MyOtherNewVariableType * myOtherVar READ myOtherVar)
	Q_PROPERTY(MyOtherNewVariableType * myOtherTwo READ myOtherTwo)

public:
	Q_INVOKABLE explicit MyNewVariableType(QUaServer *server);

	QUaBaseDataVariable * myVar();
	QUaBaseObject       * myObj();
	MyOtherNewVariableType * myOtherVar();
	MyOtherNewVariableType * myOtherTwo();

};

// ---

class MyOtherNewVariableType : public QUaBaseDataVariable
{
	Q_OBJECT

	Q_PROPERTY(QUaBaseDataVariable * myVarTwo READ myVarTwo)
public:
	Q_INVOKABLE explicit MyOtherNewVariableType(QUaServer *server);

	QUaBaseDataVariable * myVarTwo();

private:

};

// ---

class MyNewVariableSubType : public MyNewVariableType
{
	Q_OBJECT

	Q_PROPERTY(QUaBaseObject * myObjSub READ myObjSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubType(QUaServer *server);

	QUaBaseObject * myObjSub();


};

// ---

class MyNewVariableSubSubType : public MyNewVariableSubType
{
	Q_OBJECT

	Q_PROPERTY(QUaBaseObject * myObjSubSub READ myObjSubSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubSubType(QUaServer *server);

	QUaBaseObject * myObjSubSub();


};


#endif // MYNEWVARIABLETYPE_H

