#ifndef MYNEWVARIABLETYPE_H
#define MYNEWVARIABLETYPE_H

#include <QOpcUaBaseDataVariable>
#include <QOpcUaBaseObject> // NOTE : if not included, meta-info does not work

class MyOtherNewVariableType;
class MyNewVariableSubType;

class MyNewVariableType : public QOpcUaBaseDataVariable, public QOpcUaServerNodeFactory<MyNewVariableType>
{
    Q_OBJECT

	Q_PROPERTY(QOpcUaBaseDataVariable * myVar      READ getMyVar     )
	Q_PROPERTY(QOpcUaBaseObject       * myObj      READ getMyObj     )
	Q_PROPERTY(MyOtherNewVariableType * myOtherVar READ getMyOtherVar)

public:
	Q_INVOKABLE explicit MyNewVariableType(QOpcUaServerNode *parent);

	QOpcUaBaseDataVariable * getMyVar();
	QOpcUaBaseObject       * getMyObj();
	MyOtherNewVariableType * getMyOtherVar();

private:
	QOpcUaBaseDataVariable * m_myVar;
	QOpcUaBaseObject       * m_myObj;
	MyOtherNewVariableType * m_myOtherVar;
	
};

// ---

class MyOtherNewVariableType : public QOpcUaBaseDataVariable
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit MyOtherNewVariableType(QOpcUaServerNode *parent);

private:

};

// ---

class MyNewVariableSubType : public MyNewVariableType
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSub READ getMyObjSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubType(QOpcUaServerNode *parent);

	QOpcUaBaseObject * getMyObjSub();

private:
	QOpcUaBaseObject * m_myObjSub;

};

// ---

class MyNewVariableSubSubType : public MyNewVariableSubType
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSubSub READ getMyObjSubSub)

public:
	explicit MyNewVariableSubSubType(QOpcUaServerNode *parent);

	QOpcUaBaseObject * getMyObjSubSub();

private:
	QOpcUaBaseObject * m_myObjSubSub;

};


#endif // MYNEWVARIABLETYPE_H

