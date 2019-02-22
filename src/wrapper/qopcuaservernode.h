#ifndef QOPCUASERVERNODE_H
#define QOPCUASERVERNODE_H

#include <functional>
#include <typeinfo>

#include <QObject>
#include <QVariant>

#include "open62541.h"

class QOpcUaServer;
class QOpcUaProperty;
class QOpcUaBaseDataVariable;
class QOpcUaBaseObject;
class QOpcUaFolderObject;

// traits used to static assert that a method is not used
// https://stackoverflow.com/questions/24609872/delete-virtual-function-from-a-derived-class
template <typename T>
struct QOpcUaFail : std::false_type
{
};

/*
typedef struct {
	// Node Attributes
	UA_UInt32        specifiedAttributes;
	UA_LocalizedText displayName;
	UA_LocalizedText description;
	UA_UInt32        writeMask;
	UA_UInt32        userWriteMask;
} UA_NodeAttributes;
*/

//// TODO : try ?
//// https://stackoverflow.com/questions/4642079/function-signature-like-expressions-as-c-template-arguments
//template <typename T> class MyFunction;
//template <typename Ret, typename ...Arg> class MyFunction<Ret(Arg...)> {
//	/* ... */
//};

typedef QPair<quint16, QString> QBrowseName;

class QOpcUaServerNode : public QObject
{
    Q_OBJECT

	// Node Attributes

	// N/A
	//Q_PROPERTY(quint32 specifiedAttributes READ get_specifiedAttributes)

	Q_PROPERTY(QString displayName READ get_displayName WRITE set_displayName)
	Q_PROPERTY(QString description READ get_description WRITE set_description)
	Q_PROPERTY(quint32 writeMask   READ get_writeMask   WRITE set_writeMask  )

	// Cannot be read, since the local "admin" user always has full rights.
	// Cannot be written from the server, as they are specific to the different users and set by the access control callback.
	//Q_PROPERTY(quint32 userWriteMask READ get_userWriteMask)

	// Node Specifics

	// Cannot be changed once a node has been created.
	Q_PROPERTY(QString nodeId     READ get_nodeId   )
	Q_PROPERTY(QString nodeClass  READ get_nodeClass)

	// Other

    Q_PROPERTY(QBrowseName browseName READ get_browseName WRITE set_browseName)

public:
	explicit QOpcUaServerNode(QOpcUaServerNode *parent);

	// OPC UA methods API

	QString get_displayName() const;
	void    set_displayName(const QString &displayName);
	QString get_description() const;
	void    set_description(const QString &description);
	quint32 get_writeMask  () const;
	void    set_writeMask  (const quint32 &writeMask);

	QString get_nodeId     () const;
	QString get_nodeClass  () const;
	
	QPair<quint16, QString> get_browseName() const;
    void                    set_browseName(const QBrowseName &browseName);

	// Instance Creation API

	virtual QOpcUaProperty         * addProperty        (const QString &strBrowseName = "");
	virtual QOpcUaBaseDataVariable * addBaseDataVariable(const QString &strBrowseName = "");
	virtual QOpcUaBaseObject       * addBaseObject      (const QString &strBrowseName = "");
	virtual QOpcUaFolderObject     * addFolderObject    (const QString &strBrowseName = "");

	template<typename RA, typename T>
	void addMethod(const QString &strMethodName, const T &methodCallback);

	template<typename R, typename ...A>
	void addMethod(const QString &strMethodName, const std::function<R(A...)> &methodCallback);



	// private?

	// to be able to reuse methods in subclasses
	QOpcUaServer * m_qopcuaserver;

	// protected?

	// INSTANCE NodeId
	UA_NodeId m_nodeId;


protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaServerNode(QOpcUaServer *server);

private:

	template<typename R, typename ...A>
	void addMethodInternal(const QString &strMethodName, const std::function<R(A...)> &methodCallback);

	template<typename A>
	QString processArgType();
};

template<typename RA, typename T>
inline void QOpcUaServerNode::addMethod(const QString & strMethodName, const T &methodCallback)
{
	return this->addMethodInternal(strMethodName, (std::function<RA>)methodCallback);
}

template<typename R, typename ...A>
inline void QOpcUaServerNode::addMethod(const QString & strMethodName, const std::function<R(A...)> &methodCallback)
{
	return this->addMethodInternal(strMethodName, methodCallback);
}

template<typename R, typename ...A>
inline void QOpcUaServerNode::addMethodInternal(const QString & strMethodName, const std::function<R(A...)> &methodCallback)
{
	qDebug() << "Result Type" << typeid(R).name();
	QList<QString> listArgTypes = { this->processArgType<A>()... };
	qDebug() << "Arg Types" << listArgTypes;
}

template<typename A>
inline QString QOpcUaServerNode::processArgType()
{
	return QString(typeid(A).name());
}


#endif // QOPCUASERVERNODE_H

