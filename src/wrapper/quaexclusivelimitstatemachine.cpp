#include "quaexclusivelimitstatemachine.h"

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#include <QUaServer>

QUaExclusiveLimitStateMachine::QUaExclusiveLimitStateMachine(
	QUaServer* server
) : QUaFiniteStateMachine(server)
{

}

QUaExclusiveLimitState QUaExclusiveLimitStateMachine::exclusiveLimitState() const
{
	QUaLocalizedText state = this->currentState();
	return QUaExclusiveLimitState(state.text());
}

void QUaExclusiveLimitStateMachine::setExclusiveLimitState(const QUaExclusiveLimitState& exclusiveLimitState)
{
	// NOTE : check on whether a state is available is actually implemented
	// in the owning alarm (QUaLimitAlarm) because it is used to instantiate 
	// the properties (actual values) that hold the limits
	auto oldState = this->exclusiveLimitState();
	if (exclusiveLimitState == oldState)
	{
		return;
	}
	// human readible state
	this->setCurrentState(exclusiveLimitState.toString());
	// check is need to set last transition
	QUaExclusiveLimitTransition transition;
	if (
		oldState == QUa::ExclusiveLimitState::High &&
		exclusiveLimitState == QUa::ExclusiveLimitState::HighHigh
		)
	{
		transition = QUa::ExclusiveLimitTransition::HighToHighHigh;
	}
	else if (
		oldState == QUa::ExclusiveLimitState::HighHigh &&
		exclusiveLimitState == QUa::ExclusiveLimitState::High)
	{
		transition = QUa::ExclusiveLimitTransition::HighHighToHigh;
	}
	else if (
		oldState == QUa::ExclusiveLimitState::Low &&
		exclusiveLimitState == QUa::ExclusiveLimitState::LowLow)
	{
		transition = QUa::ExclusiveLimitTransition::LowToLowLow;
	}
	else if (
		oldState == QUa::ExclusiveLimitState::LowLow &&
		exclusiveLimitState == QUa::ExclusiveLimitState::Low)
	{
		transition = QUa::ExclusiveLimitTransition::LowLowToLow;
	}
	this->setLastExclusiveLimitTransition(transition);
	// emit signals
	emit this->exclusiveLimitStateChanged(exclusiveLimitState);
}

QUaExclusiveLimitTransition QUaExclusiveLimitStateMachine::lastExclusiveLimitTransition() const
{
	QUaLocalizedText transition = this->lastTransition();
	return QUaExclusiveLimitTransition(transition.text());
}

void QUaExclusiveLimitStateMachine::setLastExclusiveLimitTransition(const QUaExclusiveLimitTransition& lastExclusiveLimitTransition)
{
	auto oldTransition = this->lastExclusiveLimitTransition();
	if (lastExclusiveLimitTransition == oldTransition)
	{
		return;
	}
	// human readible state
	this->setLastTransition(lastExclusiveLimitTransition.toString());
}

void QUaExclusiveLimitStateMachine::setHighHighLimitRequired(const bool& highHighLimitRequired)
{
	// update available states and transations in state machine
	auto states = this->availableStates();
	auto transitions = this->availableTransitions();
	if (highHighLimitRequired)
	{
		Q_ASSERT(!states.contains(QUaExclusiveLimitStateMachine::stateHighHighNodeId()));
		states.append(QUaExclusiveLimitStateMachine::stateHighHighNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateHighNodeId()))
		{
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId()));
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId()));
			transitions.append(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId());
			transitions.append(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId());
		}
	}
	else
	{
		Q_ASSERT(states.contains(QUaExclusiveLimitStateMachine::stateHighHighNodeId()));
		states.removeOne(QUaExclusiveLimitStateMachine::stateHighHighNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateHighNodeId()))
		{
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId()));
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId()));
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId());
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId());
		}
	}
	this->setAvailableStates(states);
	this->setAvailableTransitions(transitions);
}


void QUaExclusiveLimitStateMachine::setHighLimitRequired(const bool& highLimitRequired)
{
	// update available states and transations in state machine
	auto states = this->availableStates();
	auto transitions = this->availableTransitions();
	if (highLimitRequired)
	{
		Q_ASSERT(!states.contains(QUaExclusiveLimitStateMachine::stateHighNodeId()));
		states.append(QUaExclusiveLimitStateMachine::stateHighNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateHighHighNodeId()))
		{
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId()));
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId()));
			transitions.append(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId());
			transitions.append(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId());
		}
	}
	else
	{
		Q_ASSERT(states.contains(QUaExclusiveLimitStateMachine::stateHighNodeId()));
		states.removeOne(QUaExclusiveLimitStateMachine::stateHighNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateHighHighNodeId()))
		{
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId()));
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId()));
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId());
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId());
		}
	}
	this->setAvailableStates(states);
	this->setAvailableTransitions(transitions);
}

void QUaExclusiveLimitStateMachine::setLowLimitRequired(const bool& lowLimitRequired)
{
	// update available states and transations in state machine
	auto states = this->availableStates();
	auto transitions = this->availableTransitions();
	if (lowLimitRequired)
	{
		Q_ASSERT(!states.contains(QUaExclusiveLimitStateMachine::stateLowNodeId()));
		states.append(QUaExclusiveLimitStateMachine::stateLowNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateLowLowNodeId()))
		{
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId()));
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId()));
			transitions.append(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId());
			transitions.append(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId());
		}
	}
	else
	{
		Q_ASSERT(states.contains(QUaExclusiveLimitStateMachine::stateLowNodeId()));
		states.removeOne(QUaExclusiveLimitStateMachine::stateLowNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateLowLowNodeId()))
		{
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId()));
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId()));
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId());
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId());
		}
	}
	this->setAvailableStates(states);
	this->setAvailableTransitions(transitions);
}

void QUaExclusiveLimitStateMachine::setLowLowLimitRequired(const bool& lowLowLimitRequired)
{
	// update available states and transations in state machine
	auto states = this->availableStates();
	auto transitions = this->availableTransitions();
	if (lowLowLimitRequired)
	{
		Q_ASSERT(!states.contains(QUaExclusiveLimitStateMachine::stateLowLowNodeId()));
		states.append(QUaExclusiveLimitStateMachine::stateLowLowNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateLowNodeId()))
		{
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId()));
			Q_ASSERT(!transitions.contains(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId()));
			transitions.append(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId());
			transitions.append(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId());
		}
	}
	else
	{
		Q_ASSERT(states.contains(QUaExclusiveLimitStateMachine::stateLowLowNodeId()));
		states.removeOne(QUaExclusiveLimitStateMachine::stateLowLowNodeId());
		if (states.contains(QUaExclusiveLimitStateMachine::stateLowNodeId()))
		{
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId()));
			Q_ASSERT(transitions.contains(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId()));
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId());
			transitions.removeOne(QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId());
		}
	}
	this->setAvailableStates(states);
	this->setAvailableTransitions(transitions);
}


QUaNodeId QUaExclusiveLimitStateMachine::stateHighHighNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_HIGHHIGH);
}

QUaNodeId QUaExclusiveLimitStateMachine::stateHighNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_HIGH);
}

QUaNodeId QUaExclusiveLimitStateMachine::stateLowNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_LOW);
}

QUaNodeId QUaExclusiveLimitStateMachine::stateLowLowNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_LOWLOW);
}

QUaNodeId QUaExclusiveLimitStateMachine::transitionLowToLowLowNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_LOWTOLOWLOW);
}

QUaNodeId QUaExclusiveLimitStateMachine::transitionLowLowToLowNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_LOWLOWTOLOW);
}

QUaNodeId QUaExclusiveLimitStateMachine::transitionHighToHighHighNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_HIGHTOHIGHHIGH);
}

QUaNodeId QUaExclusiveLimitStateMachine::transitionHighHighToHighNodeId()
{
	return UA_NODEID_NUMERIC(0, UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE_HIGHHIGHTOHIGH);
}

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS