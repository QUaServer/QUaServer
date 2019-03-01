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
	Q_INVOKABLE explicit MyNewVariableType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	QOpcUaBaseDataVariable * getMyVar();
	QOpcUaBaseObject       * getMyObj();
	MyOtherNewVariableType * getMyOtherVar();

private:
	QOpcUaBaseDataVariable * m_myVar;
	QOpcUaBaseObject       * m_myObj;
	MyOtherNewVariableType * m_myOtherVar;
	
};

// ---

class MyOtherNewVariableType : public QOpcUaBaseDataVariable, public QOpcUaServerNodeFactory<MyOtherNewVariableType>
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit MyOtherNewVariableType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

private:

};

// ---

class MyNewVariableSubType : public MyNewVariableType, public QOpcUaServerNodeFactory<MyNewVariableSubType>
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSub READ getMyObjSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	QOpcUaBaseObject * getMyObjSub();

private:
	QOpcUaBaseObject * m_myObjSub;

};

// ---

class MyNewVariableSubSubType : public MyNewVariableSubType, public QOpcUaServerNodeFactory<MyNewVariableSubSubType>
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSubSub READ getMyObjSubSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubSubType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	QOpcUaBaseObject * getMyObjSubSub();

private:
	QOpcUaBaseObject * m_myObjSubSub;

};


#endif // MYNEWVARIABLETYPE_H

