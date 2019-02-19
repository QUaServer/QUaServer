# Compile

The source code of the `open62541` library is contained in the files:

```
src/open62541.h
src/open62541.c
```

These files are **generated** by the *CMake* build of the [original library repository](https://github.com/open62541/open62541). They were generated and copied to this repo for convenience.

The library version of the `open62541` files used currently in this repo is `v0.3-rc4`.

# WIP

* Namespace `QOpcUaTypesConverter` : Implements helper methods to convert between OPC UA variables and Qt variables.

* Class `QOpcUaServer` : Inherits `QObject`, to emit events and so. Encapsulates a OPC UA Server instance and is used to configure, start, atop and subscribe to server events. It is also used to access the **unique** *Objects Folder* using the `get_objectsFolder` method, from which we can start manually creating a tree of objects and variables. It implements the `registerType<>` and `createInstance<>` templated methods that are used internally to create the new objects and variables.

* Class `QOpcUaServerNode` : Inherits `QObject` to make use of Qt's MetaProperty system. It provides access to the basic UA node attributes (*displayName*, *description*, and *writeMask*). It also contains and provides access to the **Node Id** and *browseName*. It's constructor accepts a mandatory parent reference (`QOpcUaServerNode *`) which is used to match the OPC UA address space with Qt's tree of objects, so the `QObject` API for tree-handling and memory management can be reused. The exception is the unique **unique** *Objects Folder* instance that upon instantiation uses a private method that accepts the server instance (`QOpcUaServer *`) as its mandatory parent reference.

* Class `QOpcUaAbstractObject` : Inherits `QOpcUaServerNode`. It implements access to the UA Object node attributes (only *eventNotifier*) **(TODO)**. All UA Object classes must derive from this class.

* Class `QOpcUaAbstractVariable` : Inherits `QOpcUaServerNode`. It implements access to the UA Variable node attributes (*value*, *dataType*, *valueRank*, *arrayDimensions*, *accessLevel*, *minimumSamplingInterval* and *historizing*). All UA Variable classes must derive from this class.

* Template Static Class `QOpcUaNodeFactory` : Used to implement *Static Polymorphism* and force all inheriting classes to declare a static member for the **Type Node Id** (the Node Id defining the *type* of the node). All UA Object and UA Variable classes must derive from this class and use its own class as the template argument.

* Class `QOpcUaBaseObject` : Inherits `QOpcUaAbstractObject` and `QOpcUaNodeFactory<QOpcUaBaseObject>`. It declares its *Type Node Id* to be `UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE)`. **(TODO)** Provides an API to add OPC UA Properties, Components and other Objects as children of this instance.

* Class `QOpcUaFolderObject` : Inherits `QOpcUaAbstractObject` and `QOpcUaNodeFactory<QOpcUaFolderObject>`. It declares its *Type Node Id* to be `UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE)`. Provides an API to add OPC UA Objects, Data Variables and other Folder Objects as children of this instance. It implements a **private** constructor used by the `QOpcUaServer` to create the **unique** *Objects Folder* instance of the server.

* Class `QOpcUaBaseVariable` : Inherits `QOpcUaAbstractVariable` and `QOpcUaNodeFactory<QOpcUaBaseVariable>`. It declares its *Type Node Id* to be `UA_NODEID_NUMERIC(0, UA_NS0ID_BASEVARIABLETYPE)`. **(TODO)** What can it be used for? What's the difference wrt `QOpcUaBaseDataVariable` below?

* Class `QOpcUaBaseDataVariable` : Inherits `QOpcUaAbstractVariable` and `QOpcUaNodeFactory<QOpcUaBaseDataVariable>`. It declares its *Type Node Id* to be `UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE)`. It can be added as children to `QOpcUaFolderObject` instances, as stand-alone variables capable of holding any UA basic data type.




---

## TODO

1. Find out how to use enums.

* <https://github.com/open62541/open62541/issues/2032>

* <https://github.com/open62541/open62541/issues/2167>

