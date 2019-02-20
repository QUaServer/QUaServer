#ifndef QOPCUABASEOBJECT_H
#define QOPCUABASEOBJECT_H

#include <QOpcUaAbstractObject>

class QOpcUaBaseDataVariable;
class QOpcUaFolderObject;

// Part 5 - 6.2 : BaseObjectType

class QOpcUaBaseObject : public QOpcUaAbstractObject, public QOpcUaNodeFactory<QOpcUaBaseObject>
{
    Q_OBJECT

public:
	explicit QOpcUaBaseObject(QOpcUaServerNode *parent);

	QOpcUaBaseObject       * addBaseObject();

	QOpcUaBaseDataVariable * addBaseDataVariable();

	QOpcUaFolderObject     * addFolderObject();


};



#endif // QOPCUABASEOBJECT_H

