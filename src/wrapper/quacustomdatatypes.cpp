#include "quacustomdatatypes.h"

QUaChangeStructureDataType::QUaChangeStructureDataType()
	: m_strNodeIdAffected(""), 
	  m_strNodeIdAffectedType(""), 
	  m_uiVerb(QUaChangeVerb::NodeAdded)
{
}

QUaChangeStructureDataType::QUaChangeStructureDataType(const QString & strNodeIdAffected, const QString & strNodeIdAffectedType, const Verb & uiVerb)
	: m_strNodeIdAffected(strNodeIdAffected), 
	  m_strNodeIdAffectedType(strNodeIdAffectedType), 
	  m_uiVerb(uiVerb)
{
}
