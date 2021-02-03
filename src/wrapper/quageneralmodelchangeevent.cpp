#include "quageneralmodelchangeevent.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

#include <QUaServer>

QUaGeneralModelChangeEvent::QUaGeneralModelChangeEvent(
	QUaServer *server
) : QUaBaseModelChangeEvent(server)
{
	m_changes = nullptr;
#ifdef UA_ENABLE_HISTORIZING
	m_historizing = false;
#endif // UA_ENABLE_HISTORIZING
}

QUaChangesList QUaGeneralModelChangeEvent::changes() const
{
	if (!m_changes)
	{
		auto m_thiz = const_cast<QUaGeneralModelChangeEvent*>(this);
		m_thiz->m_changes = m_thiz->getChanges();
	}
	QUaChangesList retList;
	QVariant varList = m_changes->value();
	if (!varList.isValid() || !varList.canConvert<QVariantList>())
	{
		return retList;
	}
	auto iter = varList.value<QSequentialIterable>();
	for (const QVariant &v : iter)
	{
		retList << v.value<QUaChangeStructureDataType>();
	}
	return retList;
}

void QUaGeneralModelChangeEvent::setChanges(const QUaChangesList & listVerbs)
{
	if (!m_changes)
	{
		m_changes = this->getChanges();
	}
	m_changes->setValue(listVerbs);
}

QUaProperty * QUaGeneralModelChangeEvent::getChanges()
{
	return this->browseChild<QUaProperty>("Changes");
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS