#ifndef QOPCUASERVERNODE_H
#define QOPCUASERVERNODE_H

#include <functional>
#include <typeinfo>

#include <QObject>
#include <QVariant>
#include <QMetaProperty>

#include "open62541.h"

class QOpcUaServer;
class QOpcUaProperty;
class QOpcUaBaseDataVariable;
class QOpcUaBaseObject;
class QOpcUaFolderObject;

#include <QOpcUaTypesConverter>

// traits used to static assert that a method is not used
// https://stackoverflow.com/questions/24609872/delete-virtual-function-from-a-derived-class
template <typename T>
struct QOpcUaFail : std::false_type
{
};

// to have UA_NodeId as a hash key
inline bool operator==(const UA_NodeId &e1, const UA_NodeId &e2)
{
	return e1.namespaceIndex == e2.namespaceIndex
		&& e1.identifierType == e2.identifierType
		&& e1.identifier.numeric == e2.identifier.numeric;
}

inline uint qHash(const UA_NodeId &key, uint seed)
{
	return qHash(key.namespaceIndex, seed) ^ qHash(key.identifierType, seed) ^ qHash(key.identifier.numeric, seed);
}

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

class QOpcUaServerNode : public QObject
{
    Q_OBJECT

	// Node Attributes

	// N/A
	//Q_PROPERTY(quint32 specifiedAttributes READ get_specifiedAttributes)

	Q_PROPERTY(QString displayName READ get_displayName WRITE set_displayName NOTIFY displayNameChanged)
	Q_PROPERTY(QString description READ get_description WRITE set_description NOTIFY descriptionChanged)
	Q_PROPERTY(quint32 writeMask   READ get_writeMask   WRITE set_writeMask   NOTIFY writeMaskChanged  )

	// Cannot be read, since the local "admin" user always has full rights.
	// Cannot be written from the server, as they are specific to the different users and set by the access control callback.
	//Q_PROPERTY(quint32 userWriteMask READ get_userWriteMask)

	// Node Specifics

	// Cannot be changed once a node has been created.
	Q_PROPERTY(QString nodeId     READ get_nodeId   )
	Q_PROPERTY(QString nodeClass  READ get_nodeClass)

	// Other

    Q_PROPERTY(QString browseName READ get_browseName WRITE set_browseName NOTIFY browseNameChanged)

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
	
	QString get_browseName() const;
    void    set_browseName(const QString &browseName);

	// Instance Creation API

	virtual QOpcUaProperty         * addProperty        ();
	virtual QOpcUaBaseDataVariable * addBaseDataVariable();
	virtual QOpcUaBaseObject       * addBaseObject      ();
	virtual QOpcUaFolderObject     * addFolderObject    ();


	// to be able to reuse methods in subclasses
	QOpcUaServer * m_qopcuaserver;

	// protected?

	// INSTANCE NodeId
	UA_NodeId m_nodeId;

	// private
	// NOTE : this is how we would declare a <template class> friend
	//template<typename T> friend class QOpcUaServerNodeFactory;
	UA_Server * getUAServer();

signals:

	void displayNameChanged(const QString &displayName);
	void descriptionChanged(const QString &description);
	void writeMaskChanged  (const quint32 &writeMask  );
	void browseNameChanged (const QString &browseName );

protected:
	// NOTE : this private method exists so QOpcUaServer can create the UA_NS0ID_OBJECTSFOLDER instance
	explicit QOpcUaServerNode(QOpcUaServer *server);


};

template<typename T>
struct QOpcUaServerNodeFactory
{
	// For custom obj types with props
	/*
	Description : We can have instances been created programmatically (createInstance), through the network or when registering types
	          We need to "bind" such new instances to their responding C++ objects to keep the desired tree-like API.
			  To "bind" means that the UA_Node has the C++ instance reference as a context and the C++ instance has the node's UA_NodeId. 
			  It also means the C++ instance is attached to a parent C++ instance (through Qt's object tree) which corresponds to the UA Parent.
			  Note that when creating new instance through UA API, a UA constructor is called. 
			  For complex instances, the children constructors are called first and then upwards.
			  TODO : check if UA_NodeId passed, already has parent UA_NodeId as reference
			         it is likely that this is the case since UA node creation is independent on how constructors are called
			         if so, we can check if parent context has a valid C++ reference
			  0) Instances created during type registration 
			     - Proposal : can be ignored by checking their parent node class is a UA type.		  	            
			  1) One case is a new instance to be added that has no children and the parent has already been "bound".
			     - Proposal : when UA constructor is called create new C++ instance, bind it to UA and set C++ parent.
				              find a way to define UA constructor for built in basic types with known UA_NodeId (base object, etc)
				 - Problem  : need C++ type to instantiate correctly, how to pass it to UA constructor?
				              the main blocking issue is children types auto registration that relyies on QMetaObject and no types are available
							  if we instantiate manually there is no issue because we just call the template version where we can access the type
							  we could use QMetaObject::newInstance but would require the constructor to be Q_INVOKABLE 
			  2) Other case is a new instance to be added that has no children and the parent has NOT been "bound", parent UA_Node has no C++ context.
			     - Proposal  : TODO
			  3) Other case is new instance has children. Children are already instantiated and half-bound with no C++ parent defined.
			     The new instance parent has NOT been "bound", parent UA_Node has no C++ context.
				 - Proposal  : TODO
			  4) Final case is new instance has children. Children are already instantiated and half-bound with no C++ parent defined.
			     The new instance parent has been "bound", parent UA_Node has already a C++ context.
				 - Proposal  : TODO
			  5) For binding we would like that c++ children references are already asigned before the parent C++ constructor is called
			     Proposal : use QOpcUaServerNodeFactory as static polymorphism to execute code before calling the C++ constructor 
				 Problem  : this would require the parent constructor to accept a mandatory UA_NodeId as argument to be passed to 
				            QOpcUaServerNodeFactory in order to sort out c++ children references.	
	
	*/
	inline QOpcUaServerNodeFactory(/*const UA_NodeId &nodeId*/)
	{
		// intialize member of child class, in parent class constructor
		// VS2013 allows it unless an explicit value is assigned in the class definition
		// in such case, the value is assigned after the parent constructor is called and
		// before the child constructor is called
		// NOTE : this is called for every base class that inherits it.
		//        the most specific class calls is last (if inherits from it LAST)
		// to differentiate we can set the constructor arg as /*const UA_NodeId &nodeId = UA_NODEID_NULL*/
		// so only the most specific class will be non-null
		auto ptr = static_cast<T*>(this);
		//qDebug() << "PTR" << ptr->getUAServer();
		qDebug() << T::staticMetaObject.className();

		// list meta props
		auto metaObj   = T::staticMetaObject;
		int propCount  = metaObj.propertyCount();
		int propOffset = metaObj.propertyOffset();
		for (int i = propOffset; i < propCount; i++)
		{
			// check if available in meta-system
			QMetaProperty metaproperty = metaObj.property(i);
			// check if prop type registered, register of not
			QString   strPropName = QString(metaproperty.name());
			qDebug() << strPropName;
		}

		int constCount  = metaObj.constructorCount();
		for (int i = 0; i < constCount; i++)
		{
			// print constructor signature
			QMetaMethod constructor = metaObj.constructor(i);
			qDebug() << constructor.methodSignature();
		}
	}
};

#endif // QOPCUASERVERNODE_H

