#include "quaoptionsetvariable.h"

#ifdef UA_GENERATED_NAMESPACE_ZERO_FULL

#include <QUaServer>

QUaOptionSetVariable::QUaOptionSetVariable(
	QUaServer* server
) : QUaBaseDataVariable(server)
{

}

QList<QUaLocalizedText> QUaOptionSetVariable::optionSetValues() const
{
	return const_cast<QUaOptionSetVariable*>(this)->getOptionSetValues()->value<QList<QUaLocalizedText>>();
}

void QUaOptionSetVariable::setOptionSetValues(const QList<QUaLocalizedText>& optionSetValues)
{
	this->getOptionSetValues()->setValue(optionSetValues);
}

QList<bool> QUaOptionSetVariable::bitMask() const
{
	return const_cast<QUaOptionSetVariable*>(this)->getBitMask()->value<QList<bool>>();
}

void QUaOptionSetVariable::setBitMask(const QList<bool>& effectiveDisplayName)
{
	this->getBitMask()->setValue(effectiveDisplayName);
}

QUaProperty* QUaOptionSetVariable::getOptionSetValues()
{
	return this->browseChild<QUaProperty>("OptionSetValues");
}

QUaProperty* QUaOptionSetVariable::getBitMask()
{
	return this->browseChild<QUaProperty>("BitMask", true);
}

#endif // UA_GENERATED_NAMESPACE_ZERO_FULL