#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QOpcUaAbstractObject>

// Part 5 - 6.2 : BaseObjectType

class QOpcUaBaseObject : public QOpcUaAbstractObject, public QOpcUaNodeFactory<QOpcUaBaseObject>
{
    Q_OBJECT

public:
	explicit QOpcUaBaseObject(QOpcUaServerNode *parent);



};



#endif // QOPCUABASEOBJECT_H

