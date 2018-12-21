#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QOpcUaAbstractObject>

class QOpcUaBaseObject : public QOpcUaAbstractObject, public QOpcUaNodeFactory<QOpcUaBaseObject>
{
    Q_OBJECT

public:
	explicit QOpcUaBaseObject(QOpcUaServerNode *parent);

	// TODO : addBaseDataVariable

	static UA_NodeId m_typeNodeId;

protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaBaseObject(QOpcUaServer *server);
};



#endif // QOPCUABASEOBJECT_H

