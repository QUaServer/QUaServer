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
	Q_PROPERTY(MyOtherNewVariableType * myOtherTwo READ getMyOtherTwo)

public:
	Q_INVOKABLE explicit MyNewVariableType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	QOpcUaBaseDataVariable * getMyVar();
	QOpcUaBaseObject       * getMyObj();
	MyOtherNewVariableType * getMyOtherVar();
	MyOtherNewVariableType * getMyOtherTwo();

};

// ---

class MyOtherNewVariableType : public QOpcUaBaseDataVariable, public QOpcUaServerNodeFactory<MyOtherNewVariableType>
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseDataVariable * myVarTwo READ getMyVarTwo)
public:
	Q_INVOKABLE explicit MyOtherNewVariableType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	QOpcUaBaseDataVariable * getMyVarTwo();

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


};

// ---

class MyNewVariableSubSubType : public MyNewVariableSubType, public QOpcUaServerNodeFactory<MyNewVariableSubSubType>
{
	Q_OBJECT

	Q_PROPERTY(QOpcUaBaseObject * myObjSubSub READ getMyObjSubSub)

public:
	Q_INVOKABLE explicit MyNewVariableSubSubType(QOpcUaServer *server = nullptr, const UA_NodeId &nodeId = UA_NODEID_NULL);

	QOpcUaBaseObject * getMyObjSubSub();


};


#endif // MYNEWVARIABLETYPE_H

