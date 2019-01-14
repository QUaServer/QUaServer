#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QOpcUaAbstractObject>

// Part 5 - 6.2 : BaseObjectType

class QOpcUaBaseObject : public QOpcUaAbstractObject, public QOpcUaNodeFactory<QOpcUaBaseObject>
{
    Q_OBJECT

public:
	explicit QOpcUaBaseObject(QOpcUaServerNode *parent);

	// TODO : addBaseDataVariable

	
protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaBaseObject(QOpcUaServer *server);
};



#endif // QOPCUABASEOBJECT_H

