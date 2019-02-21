# Compile

**IMPORTANT** : Requires `Qt 5.7` or higher and `C++ 11`.

The source code of the `open62541` library is contained in the files:

```
src/open62541.h
src/open62541.c
```

These files are **generated** by the *CMake* build of the [original library repository](https://github.com/open62541/open62541). They were generated and copied to this repo for convenience.

The library version of the `open62541` files used currently in this repo is `v0.3-rc4`.

# WIP


https://open62541.org/doc/0.3/tutorial_server_method.html
https://gitlab.juangburgos.com/juangburgos/notes/blob/master/OpcUA.md
https://doc.qt.io/qt-5/qvariant.html#details
https://doc.qt.io/qt-5/custom-types.html
https://www.bogotobogo.com/Qt/Qt5_QVariant_meta_object_system_MetaType.php
https://stackoverflow.com/questions/9065081/how-do-i-get-the-argument-types-of-a-function-pointer-in-a-variadic-template-cla
https://stackoverflow.com/questions/24609872/delete-virtual-function-from-a-derived-class



---

## TODO

1. Create `QOpcUaObjectFactory` and `QOpcUaVariableFactory` inheriting from `QOpcUaNodeFactory` to implement *Static Polymorphism* for ObjectType node attributes and VariableType node attributes respectively. Refactor code to inherit from these instead of `QOpcUaNodeFactory` directly.

2. Find out how to use enums.

* <https://github.com/open62541/open62541/issues/2032>

* <https://github.com/open62541/open62541/issues/2167>

