#ifndef QUAOPTIONSETVARIABLE_H
#define QUAOPTIONSETVARIABLE_H

#include <QUaBaseDataVariable>

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL

class QUaOptionSetVariable : public QUaBaseDataVariable
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit QUaOptionSetVariable(
		QUaServer* server
	);

	// value : data type shall be either a numeric DataType representing a 
	// signed or unsigned integer, or a ByteString. For example, it can be the 
	// BitFieldMaskDataType.

	// children

	// 
	QList<QUaLocalizedText> optionSetValues() const;
	void setOptionSetValues(const QList<QUaLocalizedText>& optionSetValues);

	// 
	// NOTE : optional; not created until one of these methods is called
	QList<bool> bitMask() const;
	void setBitMask(const QList<bool>& effectiveDisplayName);

protected:
	// LocalizedText[]
	QUaProperty* getOptionSetValues();
	// Boolean[]
	QUaProperty* getBitMask();

};

#endif // UA_GENERATED_NAMESPACE_ZERO_FULL

#endif // QUAOPTIONSETVARIABLE_H

