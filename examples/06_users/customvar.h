#ifndef CUSTOMVAR_H
#define CUSTOMVAR_H

#include <QUaBaseDataVariable>
#include <QUaProperty>

class CustomVar : public QUaBaseDataVariable
{
    Q_OBJECT

    Q_PROPERTY(QUaProperty         * myProp READ myProp)
	Q_PROPERTY(QUaBaseDataVariable * varFoo READ varFoo)
	Q_PROPERTY(QUaBaseDataVariable * varBar READ varBar)

public:
	Q_INVOKABLE explicit CustomVar(QUaServer *server);

	QUaProperty         * myProp();
	QUaBaseDataVariable * varFoo();
	QUaBaseDataVariable * varBar();

	// Reimplement virtual method to define default user access
	// for all instances of this type
	QUaAccessLevel userAccessLevel(const QString &strUserName) override;

};

#endif // CUSTOMVAR_H

