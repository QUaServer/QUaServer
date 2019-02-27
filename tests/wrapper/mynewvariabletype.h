#ifndef MYNEWVARIABLETYPE_H
#define MYNEWVARIABLETYPE_H

#include <QOpcUaBaseDataVariable>
#include <QOpcUaBaseObject> // NOTE : if not included, meta-info does not work

class MyOtherNewVariableType;
class MyNewVariableSubType;

class MyNewVariableType : public QOpcUaBaseDataVariable
{
    Q_OBJECT

	Q_PROPERTY(QOpcUaBaseDataVariable * myVar      READ getMyVar     )
	Q_PROPERTY(QOpcUaBaseObject       * myObj      READ getMyObj     )
	Q_PROPERTY(MyOtherNewVariableType * myOtherVar READ getMyOtherVar)
	// NOTE : Forbidden Component
	Q_PROPERTY(MyNewVariableSubType   * myProhib   READ getMyProhib  )

public:
	explicit MyNewVariableType(QOpcUaServerNode *parent);

	QOpcUaBaseDataVariable * getMyVar();
	QOpcUaBaseObject       * getMyObj();
	MyOtherNewVariableType * getMyOtherVar();
	// NOTE : Forbidden Component
	MyNewVariableSubType   * getMyProhib();

private:
	QOpcUaBaseDataVariable * m_myVar;
	QOpcUaBaseObject       * m_myObj;
	MyOtherNewVariableType * m_myOtherVar;
	// NOTE : Forbidden Component
	MyNewVariableSubType   * m_myProhib;
};

// ---

class MyOtherNewVariableType : public QOpcUaBaseDataVariable
{
	Q_OBJECT

public:
	explicit MyOtherNewVariableType(QOpcUaServerNode *parent);

private:

};

// ---

class MyNewVariableSubType : public MyNewVariableType
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSub READ getMyObjSub)

public:
	explicit MyNewVariableSubType(QOpcUaServerNode *parent);

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

