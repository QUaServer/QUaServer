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

// traits used to identify method arg and result types
// https://stackoverflow.com/questions/9065081/how-do-i-get-the-argument-types-of-a-function-pointer-in-a-variadic-template-cla
template<typename T>
struct QOpcUaMethodTraits;

template<typename R, typename ...Args>
struct QOpcUaMethodTraits<std::function<R(Args...)>>
{
	static const size_t nargs = sizeof...(Args);

	typedef R res_type;

	template <size_t i>
	struct arg
	{
		typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
	};
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

	template<typename T>
	void addMethod(const QString &strMethodName, T methodCallback);

	template<typename R, typename ...A>
	void addMethod(const QString &strMethodName, std::function<R(A...)> methodCallback);

	// private?

	// to be able to reuse methods in subclasses
	QOpcUaServer * m_qopcuaserver;

	// protected?

	// INSTANCE NodeId
	UA_NodeId m_nodeId;


protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaServerNode(QOpcUaServer *server);
};

template<typename T>
inline void QOpcUaServerNode::addMethod(const QString & strMethodName, T methodCallback)
{
	qDebug() << "T";
	qDebug() << "Result Type" << typeid(QOpcUaMethodTraits<T>::res_type).name();
	qDebug() << "Arg" << 0 << "Type" << typeid(QOpcUaMethodTraits<T>::arg<0>::type).name();
}

template<typename R, typename ...A>
inline void QOpcUaServerNode::addMethod(const QString & strMethodName, std::function<R(A...)> methodCallback)
{
	qDebug() << "R(A...)";
	qDebug() << "Result Type" << typeid(QOpcUaMethodTraits<std::function<R(A...)>>::res_type).name();
	qDebug() << "Arg" << 0 << "Type" << typeid(QOpcUaMethodTraits<std::function<R(A...)>>::arg<0>::type).name();
}


#endif // QOPCUASERVERNODE_H

