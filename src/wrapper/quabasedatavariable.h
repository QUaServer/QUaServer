#ifndef QUABASEDATAVARIABLE_H
#define QUABASEDATAVARIABLE_H

#include <QUaBaseVariable>

class QUaBaseDataVariable : public QUaBaseVariable
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaBaseDataVariable(
		QUaServer* server
	);

	// Instance Creation API
    // NOTE : implemented in qopcuaserver.h to avoid compiler errors
	template<typename T>
	T* addChild(const QUaQualifiedName &browseName, const QUaNodeId &nodeId = QUaNodeId());

};

#endif // QUABASEDATAVARIABLE_H

