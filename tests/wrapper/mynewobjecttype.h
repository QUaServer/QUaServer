#ifndef MYNEWOBJECTTYPE_H
#define MYNEWOBJECTTYPE_H

#include <QUaBaseObject>
#include <QUaProperty>
#include "mynewvariabletype.h"

class MyNewObjectType : public QUaBaseObject
{
    Q_OBJECT

	Q_PROPERTY(QUaBaseDataVariable     * myVar          READ myVar         )
	Q_PROPERTY(MyNewVariableSubSubType * myVarSubSub    READ myVarSubSub   )
	Q_PROPERTY(TestEnum                  myEnum         READ myEnum        )

public:
	Q_INVOKABLE explicit MyNewObjectType(QUaServer *server);

	enum TestEnum
	{
		NONE = 0,
		ONE  = 1,
		TWO  = 2,
		TRES = 3,
		FOUR = 4,
		FIVE = 5,
		SEIS = 6
	};
	Q_ENUM(TestEnum)

	QUaBaseDataVariable  * myVar();
	MyNewVariableSubSubType * myVarSubSub();
	QUaBaseDataVariable  * myEnumVariable();
	TestEnum                  myEnum();

	Q_INVOKABLE bool    updateMyVar(quint32 newVarVal);
	Q_INVOKABLE QString saluteName (QString strName);
	Q_INVOKABLE double  divideNums (int intNum, int intDen);


private:
	
};

class MyOtherNewObjectType : public QUaBaseObject
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit MyOtherNewObjectType(QUaServer *server);

private:


};

class MyNewObjectSubType : public MyNewObjectType
{
	Q_OBJECT

	Q_PROPERTY(QUaProperty * myProp READ myProp)

public:
	Q_INVOKABLE explicit MyNewObjectSubType(QUaServer *server);

	QUaProperty * myProp();

	Q_INVOKABLE QString concatArgs(int intNum, double dblNum, QString strName);

private:


};


#endif // MYNEWOBJECTTYPE_H

