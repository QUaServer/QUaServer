#ifndef QOPCUANODEFACTORY_H
#define QOPCUANODEFACTORY_H

#include <QOpcUaServerNode>

template <typename T>
struct QOpcUaObjectTypeTraits
{
	static QString GetBrowseName()
	{
		return QString(T::staticMetaObject.className());
	}

	// node attributes

	static QString GetDisplayName()
	{
		return QString(T::staticMetaObject.className());
	}

	static QString GetDescription()
	{
		return T::staticMetaObject.superClass() ? QString("Derived from %1").arg(T::staticMetaObject.superClass()->className()) : "";
	}

	static quint32 GetWriteMask()
	{
		return 0;
	}

	// object type attributes

	static bool GetIsAbstract()
	{
		return false;
	}
};

template <typename T>
struct QOpcUaVariableTypeTraits
{
	static QString GetBrowseName()
	{
		return QString(T::staticMetaObject.className());
	}

	// node attributes

	static QString GetDisplayName()
	{
		return QString(T::staticMetaObject.className());
	}

	static QString GetDescription()
	{
		return T::staticMetaObject.superClass() ? QString("Derived from %1").arg(T::staticMetaObject.superClass()->className()) : "";
	}

	static quint32 GetWriteMask()
	{
		return 0;
	}

	// variable type attributes

	static QVariant GetDefaultValue()
	{
		return QVariant((QVariant::Type)QMetaType::UnknownType);
	}

	static bool GetIsAbstract()
	{
		return false;
	}

};

#endif // QOPCUANODEFACTORY_H
