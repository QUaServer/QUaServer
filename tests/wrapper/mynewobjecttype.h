#ifndef MYNEWOBJECTTYPE_H
#define MYNEWOBJECTTYPE_H

#include <QOpcUaBaseObject>

class MyNewObjectType : public QOpcUaBaseObject
{
    Q_OBJECT

public:
	explicit MyNewObjectType(QOpcUaServerNode *parent);

private:

	
};

class MyOtherNewObjectType : public QOpcUaBaseObject
{
	Q_OBJECT

public:
	explicit MyOtherNewObjectType(QOpcUaServerNode *parent);

private:


};


#endif // MYNEWOBJECTTYPE_H

