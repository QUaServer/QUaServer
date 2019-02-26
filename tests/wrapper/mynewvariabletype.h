#ifndef MYNEWVARIABLETYPE_H
#define MYNEWVARIABLETYPE_H

#include <QOpcUaBaseDataVariable>

class MyNewVariableType : public QOpcUaBaseDataVariable
{
    Q_OBJECT

public:
	explicit MyNewVariableType(QOpcUaServerNode *parent);

private:

	
};

class MyOtherNewVariableType : public QOpcUaBaseDataVariable
{
	Q_OBJECT

public:
	explicit MyOtherNewVariableType(QOpcUaServerNode *parent);

private:


};

class MyNewVariableSubType : public MyNewVariableType
{
	Q_OBJECT

public:
	explicit MyNewVariableSubType(QOpcUaServerNode *parent);

private:


};

class MyNewVariableSubSubType : public MyNewVariableSubType
{
	Q_OBJECT

public:
	explicit MyNewVariableSubSubType(QOpcUaServerNode *parent);

private:


};


#endif // MYNEWVARIABLETYPE_H

